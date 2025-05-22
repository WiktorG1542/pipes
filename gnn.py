#!/usr/bin/env python3

import argparse, datetime, math, os, sys, random, time
from pathlib import Path
from typing  import Tuple, List

import pandas as pd
import torch
import torch.nn.functional as F
from   torch_geometric.data import InMemoryDataset, Data, DataLoader
from   torch_geometric.nn   import SAGEConv

# reproducibility
torch.manual_seed(42);  random.seed(42)

# helpers
TILES = {"b":0, "s":1, "l":2, "t":3}

def one_hot(idx: int, size: int = 4) -> List[float]:
    v = [0.]*size
    v[idx] = 1.
    return v

# Dataset
class PipesCsvDataset(InMemoryDataset):
    def __init__(self, csv_path: Path, transform=None, pre_transform=None,
                 split: str="train", train_frac: float=0.8):
        self.csv_path = Path(csv_path)
        self.split    = split
        self.train_frac = train_frac
        super().__init__(self.csv_path.parent,
                         transform, pre_transform)
        self.data, self.slices = torch.load(
            self.processed_paths[0], weights_only=False)        

    @property
    def processed_file_names(self):
        return [f"{self.csv_path.stem}_{self.split}.pt"]

    @property
    def raw_file_names(self):
        return []

    def download(self):
        pass

    def process(self):
        df = pd.read_csv(self.csv_path)
        # simple random train/test split
        df = df.sample(frac=1.0, random_state=42).reset_index(drop=True)
        cut = int(len(df)*self.train_frac)
        if self.split=="train": df = df.iloc[:cut]
        else:                    df = df.iloc[cut:]

        data_list=[]
        for _,row in df.iterrows():
            W,H = map(int,row["shape"].split("x"))
            uns  = row["unsolved"].strip()
            sol  = list(map(int,row["solved"].split(",")))

            # build node features
            xs=[] ; ys=[]
            for i in range(W*H):
                tile   = TILES[uns[2*i]]
                rot    = int(uns[2*i+1])

                tile_oh     = one_hot(tile,4)
                rot_oh      = one_hot(rot,4)
                sincos      = [math.sin(rot*math.pi/2),
                               math.cos(rot*math.pi/2)]
                coords_norm = [(i%W)/(W-1), (i//W)/(H-1)]
                bias        = [1.0]
                locked      = [0.]

                xs.append(tile_oh+rot_oh+sincos+coords_norm+bias+locked)
                ys.append(sol[i])

            x = torch.tensor(xs, dtype=torch.float)
            y = torch.tensor(ys, dtype=torch.long)

            # edges (4-neighbourhood)
            src=[]
            dst=[]
            for r in range(H):
                for c in range(W):
                    idx = r*W+c
                    for dr,dc in [(1,0),(-1,0),(0,1),(0,-1)]:
                        rr,cc = r+dr, c+dc
                        if 0<=rr<H and 0<=cc<W:
                            src.append(idx)
                            dst.append(rr*W+cc)
            edge_index = torch.tensor([src,dst], dtype=torch.long)

            data_list.append(Data(x=x, edge_index=edge_index, y=y))

        data, slices = self.collate(data_list)
        torch.save((data,slices), self.processed_paths[0])

# Model
class GraphSagePipes(torch.nn.Module):
    def __init__(self, in_dim:int, hidden_dim:int, num_layers:int=4):
        super().__init__()
        self.convs = torch.nn.ModuleList()
        self.convs.append(SAGEConv(in_dim, hidden_dim))
        for _ in range(num_layers-2):
            self.convs.append(SAGEConv(hidden_dim, hidden_dim))
        self.convs.append(SAGEConv(hidden_dim, 4))

    def forward(self, data: Data):
        x,edge_index = data.x, data.edge_index
        for conv in self.convs[:-1]:
            x = F.relu(conv(x,edge_index))
        x = self.convs[-1](x,edge_index)
        return x

# train / test
def train_epoch(model, loader, optimiser, device):
    model.train()
    total_loss=0
    for data in loader:
        data = data.to(device)
        optimiser.zero_grad()
        out = model(data)
        loss = F.cross_entropy(out, data.y)
        loss.backward()
        optimiser.step()
        total_loss += loss.item()*data.num_nodes
    return total_loss / len(loader.dataset)

@torch.no_grad()
def eval_epoch(model, loader, device):
    model.eval()
    correct = total_nodes = 0
    for data in loader:
        data = data.to(device)
        out  = model(data).argmax(dim=1)
        correct     += int((out == data.y).sum())
        total_nodes += data.num_nodes
    return correct / total_nodes

# inference util
def predict_single(model, unsolved:str, shape:str, device) -> List[int]:
    W,H = map(int,shape.split("x"))
    assert len(unsolved)==2*W*H
    xs=[]
    for i in range(W*H):
        tile   = TILES[unsolved[2*i]]
        rot    = int(unsolved[2*i+1])
        xs.append(one_hot(tile,4)+one_hot(rot,4)+
                  [math.sin(rot*math.pi/2), math.cos(rot*math.pi/2)]+
                  [(i%W)/(W-1), (i//W)/(H-1)]+[1.]+[0.])
    edge_src,edge_dst=[],[]
    for r in range(H):
        for c in range(W):
            idx=r*W+c
            for dr,dc in [(1,0),(-1,0),(0,1),(0,-1)]:
                rr,cc=r+dr,c+dc
                if 0<=rr<H and 0<=cc<W:
                    edge_src.append(idx); edge_dst.append(rr*W+cc)
    data=Data(x=torch.tensor(xs), edge_index=torch.tensor([edge_src,edge_dst]))
    data=data.to(device)
    model.eval(); out=model(data)
    return out.argmax(dim=1).cpu().tolist()

# CLI
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--csv", required=True)
    p.add_argument("--epochs", type=int, default=20)
    p.add_argument("--batch",  type=int, default=128)
    p.add_argument("--hidden", type=int, default=128)
    p.add_argument("--layers", type=int, default=4)
    p.add_argument("--lr",     type=float, default=1e-3)
    p.add_argument("--eval",   help="path to text file with one unsolved line")
    args=p.parse_args()

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    if args.eval:
        model = GraphSagePipes(in_dim=15, hidden_dim=args.hidden,
                               num_layers=args.layers).to(device)
        model.load_state_dict(torch.load("best_pipe_model.pt", map_location=device))
        with open(args.eval) as fh:
            line=fh.readline().strip()
        unsolved,shape=line.split(";") if ";" in line else (line,"10x10")
        pred = predict_single(model, unsolved, shape, device)
        ts = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        outf=f"prediction_{ts}.txt"
        with open(outf,"w") as fh:
            fh.write(",".join(map(str,pred)))
        print(f"Predicted rotations saved to {outf}")
        return

    # training pipeline
    train_ds=PipesCsvDataset(args.csv, split="train")
    test_ds =PipesCsvDataset(args.csv, split="test")
    train_loader=DataLoader(train_ds, batch_size=args.batch, shuffle=True)
    test_loader =DataLoader(test_ds , batch_size=args.batch, shuffle=False)

    model=GraphSagePipes(in_dim=train_ds[0].num_features,
                         hidden_dim=args.hidden, num_layers=args.layers).to(device)
    opt=torch.optim.Adam(model.parameters(), lr=args.lr)

    best=0.; best_state=None
    for epoch in range(1,args.epochs+1):
        loss=train_epoch(model,train_loader,opt,device)
        acc =eval_epoch (model,test_loader ,device)
        if acc>best:
            best=acc; best_state=model.state_dict()
        print(f"Epoch {epoch:02d} | loss {loss:.4f} | test-acc {acc*100:5.2f}%")
    torch.save(best_state,"best_pipe_model.pt")
    print(f"Done.  Best accuracy {best*100:.2f}%  model saved to best_pipe_model.pt")

if __name__=="__main__":
    main()
