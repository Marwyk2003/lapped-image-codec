import glob
import os
import re
from collections import defaultdict

import matplotlib.image as mpimg
import matplotlib.pyplot as plt


def parse_filename(filepath):
    # output/{example}/{example}_{type}_{quantizer}_{strength}_decoded.png
    basename = os.path.basename(filepath)
    match = re.match(r"(.+)_(block|lapped)_(Q\w+)_(\d+)_decoded\.png", basename)
    if match:
        return {
            "example": match.group(1),
            "type": match.group(2),
            "quantizer": match.group(3),
            "strength": int(match.group(4)),
            "path": os.path.abspath(filepath),
        }
    return None


def create_summary_by_params(data, output_dir):
    """Different examples, same parameters."""
    groups = defaultdict(list)
    for item in data:
        key = (item["type"], item["quantizer"], item["strength"])
        groups[key].append(item)

    for key, items in groups.items():
        if len(items) < 2:
            continue

        t, q, s = key
        fig, axes = plt.subplots(
            1, len(items), figsize=(5 * len(items), 5), squeeze=False
        )

        fig.suptitle(f"Type: {t}, Quantizer: {q}, Strength: {s}")

        for i, item in enumerate(sorted(items, key=lambda x: x["example"])):
            ax = axes[0, i]
            try:
                img = mpimg.imread(item["path"])
                if img.ndim == 2:
                    ax.imshow(img, cmap="gray")
                else:
                    ax.imshow(img)
                ax.set_title(item["example"])
            except Exception as e:
                ax.text(
                    0.5,
                    0.5,
                    f"Error loading\n{item['example']}",
                    ha="center",
                    va="center",
                )
                print(f"Error loading {item['path']}: {e}")
            ax.axis("off")

        output_name = os.path.join(output_dir, f"summary_params_{t}_{q}_{s}.png")
        plt.tight_layout()
        plt.savefig(output_name)
        print(f"Saved {output_name}")
        plt.close()


def create_summary_by_example(data, output_dir):
    """Different parameters, same example."""
    groups = defaultdict(list)
    for item in data:
        key = (item["example"], item["strength"])
        groups[key].append(item)

    for key, items in groups.items():
        ex, s = key

        types = sorted(list(set(item["type"] for item in items)))
        quantizers = sorted(list(set(item["quantizer"] for item in items)))

        fig, axes = plt.subplots(
            len(types),
            len(quantizers),
            figsize=(4 * len(quantizers), 4 * len(types)),
            squeeze=False,
        )
        fig.suptitle(f"Example: {ex}, Strength: {s}")

        lookup = {(item["type"], item["quantizer"]): item for item in items}

        for i, t in enumerate(types):
            for j, q in enumerate(quantizers):
                ax = axes[i, j]
                item = lookup.get((t, q))
                if item:
                    try:
                        img = mpimg.imread(item["path"])
                        if img.ndim == 2:
                            ax.imshow(img, cmap="gray")
                        else:
                            ax.imshow(img)
                        ax.set_title(f"{t}\n{q}")
                    except Exception as e:
                        ax.text(
                            0.5,
                            0.5,
                            f"Error loading\n{t}_{q}",
                            ha="center",
                            va="center",
                        )
                        print(f"Error loading {item['path']}: {e}")
                else:
                    ax.text(0.5, 0.5, "N/A", ha="center", va="center")
                ax.axis("off")

        output_name = os.path.join(output_dir, f"summary_example_{ex}_s{s}.png")
        plt.tight_layout()
        plt.savefig(output_name)
        print(f"Saved {output_name}")
        plt.close()


def create_grid_summary(
    data, filter_func, row_param, col_param, title, output_name, output_dir
):
    """Generic function to create a grid summary based on filtered data."""
    items = [item for item in data if filter_func(item)]
    if not items:
        print(f"No data found for {title}")
        return

    rows = sorted(list(set(item[row_param] for item in items)))
    cols = sorted(list(set(item[col_param] for item in items)))

    fig, axes = plt.subplots(
        len(rows),
        len(cols),
        figsize=(5 * len(cols), 5 * len(rows)),
        squeeze=False,
    )
    fig.suptitle(title)

    lookup = {(item[row_param], item[col_param]): item for item in items}

    for i, r in enumerate(rows):
        for j, c in enumerate(cols):
            ax = axes[i, j]
            item = lookup.get((r, c))
            if item:
                try:
                    img = mpimg.imread(item["path"])
                    if img.ndim == 2:
                        ax.imshow(img, cmap="gray")
                    else:
                        ax.imshow(img)
                    ax.set_title(f"{row_param}:{r}\n{col_param}:{c}")
                except Exception as e:
                    ax.text(
                        0.5, 0.5, f"Error loading\n{r}_{c}", ha="center", va="center"
                    )
                    print(f"Error loading {item['path']}: {e}")
            else:
                ax.text(0.5, 0.5, "N/A", ha="center", va="center")
            ax.axis("off")

    full_output_path = os.path.join(output_dir, output_name)
    plt.tight_layout()
    plt.savefig(full_output_path)
    print(f"Saved {full_output_path}")
    plt.close()


def main():
    files = glob.glob("output/**/*_decoded.png", recursive=True)
    data = []
    for f in files:
        parsed = parse_filename(f)
        if parsed:
            data.append(parsed)

    if not data:
        print("No decoded images found in output/")
        return

    output_dir = "summaries"
    os.makedirs(output_dir, exist_ok=True)

    print(f"Found {len(data)} images. Generating summaries...")

    # Original summaries
    # create_summary_by_params(data, output_dir)
    # create_summary_by_example(data, output_dir)

    # 1. Barbara blocked (all strengths and quantizers)
    create_grid_summary(
        data,
        lambda x: x["example"] == "barbara" and x["type"] == "block",
        "strength",
        "quantizer",
        "Barbara Blocked (Strength vs Quantizer)",
        "summary_barbara_block.png",
        output_dir,
    )

    # 2. Barbara lapped (all strengths and quantizers)
    create_grid_summary(
        data,
        lambda x: x["example"] == "barbara" and x["type"] == "lapped",
        "strength",
        "quantizer",
        "Barbara Lapped (Strength vs Quantizer)",
        "summary_barbara_lapped.png",
        output_dir,
    )

    # 3. Strength comparison for Q8Base block and lapped for Barbara
    create_grid_summary(
        data,
        lambda x: x["example"] == "barbara" and x["quantizer"] == "Q8Base",
        "strength",
        "type",
        "Barbara Q8Base (Strength vs Type)",
        "summary_barbara_q8base_strength.png",
        output_dir,
    )

    # 4. Strength 10, Q8Base, both lapped and block, both examples
    create_grid_summary(
        data,
        lambda x: x["strength"] == 10 and x["quantizer"] == "Q8Base",
        "example",
        "type",
        "Q8Base Strength 10 (Example vs Type)",
        "summary_q8base_s10_examples.png",
        output_dir,
    )


if __name__ == "__main__":
    main()
