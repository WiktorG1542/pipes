#!/usr/bin/env python3

# imports
import argparse, datetime, math, random, sys, json
from pathlib import Path
from typing import List

import pandas as pd
import torch, torch.nn.functional as F
from torch_geometric.data import InMemoryDataset, Data, DataLoader
from torch_geometric.nn   import SAGEConv

# reproducibility & constants
torch.manual_seed(42);  random.seed(42)

FEATURE_DIM = 14
TILES       = {"b": 0, "s": 1, "l": 2, "t": 3}

DIRS = [(0, -1), (1, 0), (0, 1), (-1, 0)]          # N, E, S, W
MASKS_BASE = {                                     # “rotation 0” masks
    "b": 0b0001,   # blob  ⭡
    "s": 0b0101,   # straight ⭡ ⭣
    "l": 0b0011,   # L-shape ⭡ ⭢
    "t": 0b0111,   # T-shape ⭡ ⭢ ⭣
}
def rot(mask: int, r: int) -> int:                 # rotate 90° cw r times
    r &= 3
    return ((mask << r) & 0b1111) | (mask >> (4 - r))


def board_solved(unsolved: str, rotations: list[int], shape_str: str,
                 origin: tuple[int,int] | None = None) -> bool:
    """
    • no leaks (every open pipe matches a neighbour) AND
    • water starting at `origin` (default = first cell with an opening)
      reaches every tile.
    """
    W, H = map(int, shape_str.split("x"))

    # --- build per-tile 4-bit masks after rotation -------------------------
    masks = []
    for i in range(W * H):
        tile   = unsolved[2 * i]
        rot_i  = rotations[i] & 3
        masks.append(rot(MASKS_BASE[tile], rot_i))

    # --- 1) leak-tightness --------------------------------------------------
    get = lambda r, c: masks[r * W + c]
    for r in range(H):
        for c in range(W):
            m = get(r, c)
            # iterate over four directions
            for bit, (dx, dy), opp in [
                (0b0001, (0, -1), 0b0100),   # N needs neighbour's S
                (0b0010, (1,  0), 0b1000),   # E needs neighbour's W
                (0b0100, (0,  1), 0b0001),   # S needs neighbour's N
                (0b1000, (-1, 0), 0b0010),   # W needs neighbour's E
            ]:
                if m & bit:                                  # pipe open here
                    nx, ny = c + dx, r + dy
                    if not (0 <= nx < W and 0 <= ny < H):    # board edge = leak
                        return False
                    if not (get(ny, nx) & opp):              # mismatch = leak
                        return False

    # --- 2) connectivity / reachability ------------------------------------
    # pick origin if not provided: first tile that has at least one opening
    if origin is None:
        flat_idx = next(i for i, m in enumerate(masks) if m)   # must exist
        origin = (flat_idx % W, flat_idx // W)

    sx, sy = origin
    seen = set()
    stack = [(sx, sy)]
    while stack:
        x, y = stack.pop()
        if (x, y) in seen:
            continue
        seen.add((x, y))
        m = get(y, x)
        # explore through open edges
        for bit, (dx, dy), opp in [
            (0b0001, (0, -1), 0b0100),
            (0b0010, (1,  0), 0b1000),
            (0b0100, (0,  1), 0b0001),
            (0b1000, (-1, 0), 0b0010),
        ]:
            if m & bit:
                nx, ny = x + dx, y + dy
                if 0 <= nx < W and 0 <= ny < H and (get(ny, nx) & opp):
                    stack.append((nx, ny))

    return len(seen) == W * H

def one_hot(i: int, size: int = 4) -> List[float]:
    v = [0.0]*size
    v[i] = 1.0
    return v

# dataset
class PipesCsvDataset(InMemoryDataset):
    def __init__(self, csv: Path, split="train", train_frac=0.8,
                 transform=None, pre_transform=None):
        self.csv_path   = Path(csv)
        self.split      = split
        self.train_frac = train_frac
        super().__init__(self.csv_path.parent, transform, pre_transform)
        self.data, self.slices = torch.load(self.processed_paths[0], weights_only=False)

    @property
    def processed_file_names(self): return [f"{self.csv_path.stem}_{self.split}.pt"]
    @property
    def raw_file_names(self):       return []
    def download(self):             pass

    def process(self):
        df  = pd.read_csv(self.csv_path)
        df  = df.sample(frac=1.0, random_state=42)
        cut = int(len(df)*self.train_frac)
        df  = df.iloc[:cut] if self.split=="train" else df.iloc[cut:]

        data_list = []
        for _, row in df.iterrows():
            W, H  = map(int, row["shape"].split("x"))
            uns   = row["unsolved"].strip()
            sol   = list(map(int, row["solved"].split(",")))

            x_lst, y_lst = [], []
            for i in range(W*H):
                tile = TILES[uns[2*i]]
                rot  = int(uns[2*i+1])

                feats = ( one_hot(tile) + one_hot(rot) +
                          [math.sin(rot*math.pi/2), math.cos(rot*math.pi/2)] +
                          [(i%W)/(W-1), (i//W)/(H-1)] + [1.0, 0.0] )

                x_lst.append(feats)
                y_lst.append(sol[i])

            x = torch.tensor(x_lst, dtype=torch.float)
            y = torch.tensor(y_lst, dtype=torch.long)

            src, dst = [], []
            for r in range(H):
                for c in range(W):
                    v = r*W+c
                    for dr,dc in [(1,0),(-1,0),(0,1),(0,-1)]:
                        rr,cc = r+dr, c+dc
                        if 0<=rr<H and 0<=cc<W:
                            src.append(v); dst.append(rr*W+cc)
            edge_index = torch.tensor([src, dst], dtype=torch.long)

            d = Data(x=x, edge_index=edge_index, y=y)
            d.unsolved  = uns
            d.shape_str = row["shape"]
            data_list.append(d)

        data, slices = self.collate(data_list)
        torch.save((data, slices), self.processed_paths[0])

# model
class GraphSagePipes(torch.nn.Module):
    """SAGE([FEATURE_DIM] → hidden → … → 4)"""
    def __init__(self, hidden: int = 128, layers: int = 4):
        super().__init__()
        self.convs = torch.nn.ModuleList()
        self.convs.append(SAGEConv(FEATURE_DIM, hidden))
        for _ in range(layers-2):
            self.convs.append(SAGEConv(hidden, hidden))
        self.convs.append(SAGEConv(hidden, 4))

    def forward(self, data: Data):
        x, ei = data.x, data.edge_index
        for conv in self.convs[:-1]:
            x = F.relu(conv(x, ei))
        return self.convs[-1](x, ei)

# train / evaluate helpers
def train_epoch(model, loader, opt, device):
    model.train(); tot = 0
    for d in loader:
        d = d.to(device)
        opt.zero_grad()
        loss = F.cross_entropy(model(d), d.y)
        loss.backward(); opt.step()
        tot += loss.item()*d.num_nodes
    return tot/len(loader.dataset)

@torch.no_grad()
def eval_epoch(model, loader, device):
    def rebuild(x): return ''.join(x) if isinstance(x, list) else x

    model.eval()
    solved_boards = 0
    correct_nodes = 0
    total_nodes   = 0
    for batch in loader:
        batch = batch.to(device)
        out   = model(batch)
        pred  = out.argmax(1).cpu()

        # ---- node-level ---------------------------------------------------
        correct_nodes += int((pred == batch.y).sum())
        total_nodes   += batch.num_nodes

        # ---- board-level --------------------------------------------------
        uns = rebuild(batch.unsolved)
        shp = rebuild(batch.shape_str)
        if board_solved(uns, pred.tolist(), shp):
            solved_boards += 1

    node_acc   = correct_nodes / total_nodes
    board_acc  = solved_boards / len(loader)
    return node_acc, board_acc

# single-board helper
@torch.no_grad()
def predict_single(model, unsolved: str, shape: str, device) -> List[int]:
    W,H = map(int, shape.split("x"))
    feats, src, dst = [], [], []
    for i in range(W*H):
        tile = TILES[unsolved[2*i]]
        rot  = int(unsolved[2*i+1])
        feats.append(one_hot(tile)+one_hot(rot)+
                     [math.sin(rot*math.pi/2), math.cos(rot*math.pi/2)]+
                     [(i%W)/(W-1), (i//W)/(H-1)] + [1.0,0.0])
    for r in range(H):
        for c in range(W):
            v=r*W+c
            for dr,dc in [(1,0),(-1,0),(0,1),(0,-1)]:
                rr,cc=r+dr,c+dc
                if 0<=rr<H and 0<=cc<W:
                    src.append(v); dst.append(rr*W+cc)
    data = Data(x=torch.tensor(feats), edge_index=torch.tensor([src,dst]))
    out  = model(data.to(device)).argmax(1).cpu().tolist()
    return out

# CLI
def main():
    par = argparse.ArgumentParser()
    sub = par.add_subparsers(dest="mode", required=True)

    tr = sub.add_parser("train")
    tr.add_argument("--csv", required=True)
    tr.add_argument("--epochs", type=int, default=20)
    tr.add_argument("--batch",  type=int, default=128)
    tr.add_argument("--hidden", type=int, default=128)
    tr.add_argument("--layers", type=int, default=4)
    tr.add_argument("--lr",     type=float, default=1e-3)

    ev = sub.add_parser("eval")
    ev.add_argument("puzzle_txt")
    ev.add_argument("--checkpoint", default="best_pipe_model.pt")

    args = par.parse_args()
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # train
    if args.mode == "train":
        train_ds = PipesCsvDataset(args.csv, split="train")
        test_ds  = PipesCsvDataset(args.csv, split="test")
        t_loader = DataLoader(train_ds, batch_size=args.batch, shuffle=True)
        v_loader = DataLoader(test_ds , batch_size=1)

        model = GraphSagePipes(hidden=args.hidden, layers=args.layers).to(device)
        opt   = torch.optim.Adam(model.parameters(), lr=args.lr)

        best_acc, best_state = 0.0, None
        for epoch in range(1, args.epochs+1):
            loss = train_epoch(model, t_loader, opt, device)
            node_acc, board_acc = eval_epoch(model, v_loader, device)
            if node_acc > best_acc:
                best_acc, best_state = node_acc, model.state_dict()
            print(f"Epoch {epoch:02d} | loss {loss:.4f} "
                f"| node-acc {node_acc*100:5.2f}% "
                f"| solved-boards {board_acc*100:4.1f}%")
        torch.save({
            "state_dict": best_state,
            "hidden": args.hidden,
            "layers": args.layers
        }, "best_pipe_model.pt")
        print(f"✓ saved best_pipe_model.pt  (accuracy {best_acc*100:.2f} %)")
        return

    # eval
    ckpt   = torch.load(args.checkpoint, map_location=device)
    model  = GraphSagePipes(hidden=ckpt["hidden"], layers=ckpt["layers"]).to(device)
    model.load_state_dict(ckpt["state_dict"])

    line = Path(args.puzzle_txt).read_text().strip()
    unsolved, shape = (line.split(";")+["10x10"])[:2]
    pred   = predict_single(model, unsolved, shape, device)

    ts   = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    out  = f"prediction_{ts}.txt"
    Path(out).write_text(",".join(map(str, pred)))
    print(f"✓ rotations written to  {out}")

if __name__ == "__main__":
    main()
