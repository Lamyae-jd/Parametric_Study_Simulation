"""Retrain small_v2 on the mono-energy runs of the study (with K fluorescence) — the recipe of the original student
(task phase, no distillation), identical to the LGSO retraining (LGSO 5x5 windows/reconstruction_small_v2):

  StudentNet(in_ch=3, widths=(16, 32, 32)), BN + ReLU6, 2 338 parameters, random init (seed 1337)
  FocalLoss(γ=2) on the primary map; AdamW lr 5e-4, weight decay 1e-4, batch 1 024; linear warmup 3 epochs;
  ReduceLROnPlateau(max, ×0.5, patience 5) on the validation score; ≤ 60 epochs, early stop after 18 without gain
  augmentation: one of the 8 symmetries of the 5x5 square per sample
  score = AUPRC_Primary_ICS on the validation photons (common.ics_metrics)

Usage: /usr/bin/python3 scripts/train.py
"""

import json
import time

import numpy as np
import torch

import s05_common as c
from pi_kd.losses.focal_loss import FocalLoss  # noqa: E402  (GNN_DIR on sys.path via common)
from pi_kd.models.student import StudentNet  # noqa: E402

DEVICE = "cuda"
WIDTHS = (16, 32, 32)
LR, WEIGHT_DECAY, BATCH = 5e-4, 1e-4, 1024
EPOCHS, EARLY_PATIENCE, WARMUP = 60, 18, 3
SEED = 1337


def load(name: str) -> tuple:
    d = np.load(c.DATA_DIR / f"{name}.npz")
    return torch.from_numpy(d["x"]).to(DEVICE), torch.from_numpy(d["target"]).to(DEVICE), d["is_ics"]


def augment(x: torch.Tensor, target: torch.Tensor) -> tuple:
    """Independent random D4 symmetry per sample, applied identically to the 3 channels and the target."""
    t = target.view(-1, 1, 5, 5)
    sym = torch.randint(0, 8, (len(x),), device=x.device)
    out_x, out_t = x.clone(), t.clone()
    for s in range(8):
        idx = (sym == s).nonzero(as_tuple=True)[0]
        if len(idx) == 0:
            continue
        xs, ts = x[idx], t[idx]
        if s >= 4:
            xs, ts = torch.flip(xs, dims=(3,)), torch.flip(ts, dims=(3,))
        k = s % 4
        out_x[idx], out_t[idx] = torch.rot90(xs, k, dims=(2, 3)), torch.rot90(ts, k, dims=(2, 3))
    return out_x, out_t


@torch.no_grad()
def val_score(model, x_val: torch.Tensor, target_val: torch.Tensor, ics_val: np.ndarray) -> dict:
    model.eval()
    idx = torch.from_numpy(np.flatnonzero(ics_val)).to(DEVICE)
    logits = torch.cat([model(x_val[idx[i:i + 16384]])["P_logits"].flatten(1).float()
                        for i in range(0, len(idx), 16384)]).cpu().numpy()
    metrics, _ = c.qcommon.ics_metrics(logits, target_val[idx].cpu().numpy())
    return metrics


def main():
    torch.manual_seed(SEED)
    np.random.seed(SEED)
    x_tr, t_tr, _ = load("train")
    x_va, t_va, ics_va = load("val")
    model = StudentNet(in_ch=3, widths=WIDTHS).to(DEVICE)
    n_params = sum(p.numel() for p in model.parameters())
    print(f"StudentNet {WIDTHS}: {n_params} params | train {len(x_tr):,} | val {len(x_va):,} (ICS {ics_va.sum():,})",
          flush=True)
    loss_fn = FocalLoss(gamma=2.0)
    opt = torch.optim.AdamW(model.parameters(), lr=LR, weight_decay=WEIGHT_DECAY)
    plateau = torch.optim.lr_scheduler.ReduceLROnPlateau(opt, mode="max", factor=0.5, patience=5)
    best, bad, history = -1.0, 0, []
    c.RETRAINED_CKPT.parent.mkdir(exist_ok=True)
    for epoch in range(EPOCHS):
        if epoch < WARMUP:
            for g in opt.param_groups:
                g["lr"] = LR * (epoch + 1) / WARMUP
        model.train()
        t0, losses = time.time(), []
        for idx in torch.randperm(len(x_tr), device=DEVICE).split(BATCH):
            xb, tb = augment(x_tr[idx], t_tr[idx])
            loss = loss_fn(model(xb)["P_logits"], tb)
            opt.zero_grad()
            loss.backward()
            opt.step()
            losses.append(loss.item())
        m = val_score(model, x_va, t_va, ics_va)
        score = m["AUPRC_Primary_ICS"]
        if epoch >= WARMUP:
            plateau.step(score)
        improved = score > best
        if improved:
            best, bad = score, 0
            torch.save({"student": model.state_dict(), "cfg": {"student": {"in_ch": 3, "widths": list(WIDTHS)}},
                        "epoch": epoch, "metrics": m}, c.RETRAINED_CKPT)
        else:
            bad += 1
        history.append({"epoch": epoch, "loss": float(np.mean(losses)), "lr": opt.param_groups[0]["lr"], **m})
        print(f"ep {epoch:2d} | loss {np.mean(losses):.5f} | lr {opt.param_groups[0]['lr']:.1e} | " +
              " ".join(f"{k} {v:.4f}" for k, v in m.items()) + f" | {time.time() - t0:.0f}s{' *' if improved else ''}",
              flush=True)
        (c.LOGS_DIR / "train_history.json").write_text(json.dumps(history, indent=1))
        if bad >= EARLY_PATIENCE:
            print("arrêt anticipé", flush=True)
            break
    print(f"meilleur AUPRC_Primary_ICS (val) {best:.4f} → {c.RETRAINED_CKPT}", flush=True)


if __name__ == "__main__":
    main()
