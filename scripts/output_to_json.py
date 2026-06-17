#!/usr/bin/env python3
"""
Convert gammacombo CL interval output files (.py) to structured JSON
suitable for upload to Zenodo.

Usage:
    python scripts/output_to_json.py -i alphacombo/plots/cl/clintervals_hflav_pdg2025_alpha_Prob.py -o zenodo/alpha_wa_outputs.json
    python scripts/output_to_json.py -i gammacombo/plots/cl/clintervals_hflav_pdg2025_g_Prob.py -o zenodo/gamma_wa_outputs.json
"""

import argparse
import json
import sys

# The CL values printed by gammacombo are rounded;
# these are the true values (1, 2, 3 sigma two-sided).
CL_MAP = {
    "0.68": 0.683,
    "0.95": 0.954,
    "1.00": 0.997,
}


def convert_intervals(input_path):
    """Load a gammacombo CL intervals .py file and return structured dict."""
    ns = {}
    with open(input_path) as f:
        exec(f.read(), ns)
    intervals = ns["intervals"]

    output = {"confidence_intervals": []}
    for cl_key, entries in intervals.items():
        for e in entries:
            output["confidence_intervals"].append({
                "variable": e["var"],
                "central_value": float(e["central"]),
                "interval_min": float(e["min"]),
                "interval_max": float(e["max"]),
                "error_neg": float(e["neg"]),
                "error_pos": float(e["pos"]),
                "confidence_level": CL_MAP.get(e["cl"], float(e["cl"])),
                "unit": e["unit"],
                "method": e["method"],
            })
    return output


def main():
    parser = argparse.ArgumentParser(
        description="Convert gammacombo CL interval .py files to JSON"
    )
    parser.add_argument(
        "-i", "--input", required=True, help="Input .py CL intervals file"
    )
    parser.add_argument(
        "-o", "--output", required=True, help="Output JSON file path"
    )
    args = parser.parse_args()

    result = convert_intervals(args.input)

    with open(args.output, "w") as f:
        json.dump(result, f, indent=2)

    n = len(result["confidence_intervals"])
    print(f"Wrote {args.output}: {n} confidence intervals", file=sys.stderr)


if __name__ == "__main__":
    main()
