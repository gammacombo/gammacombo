#!/usr/bin/env python3
"""
Parse the output of gammacombo --info into a structured JSON format
suitable for upload to Zenodo.

Usage:
    bin/alpha_wa -c 50 -v --info 2>&1 > raw.txt
    python scripts/info_to_json.py -i raw.txt -o zenodo/alpha_wa_inputs.json -c 50 --source alphacombo/main/alpha_wa.cpp

    bin/gamma_wa -c 360 -v --info 2>&1 > raw.txt
    python scripts/info_to_json.py -i raw.txt -o zenodo/gamma_wa_inputs.json -c 360 --source gammacombo/main/gamma_wa.cpp
"""

import argparse
import json
import re
import sys


def parse_observable_line(line):
    """Parse a line like: obs_name  =  1.51900 +/- 0.00400 +/- 0.00000"""
    m = re.match(
        r"\s+(\S+)\s+=\s+([\d.eE+\-]+)\s+\+/-\s+([\d.eE+\-]+)\s+\+/-\s+([\d.eE+\-]+)",
        line,
    )
    if m:
        return {
            "name": m.group(1),
            "value": float(m.group(2)),
            "stat_error": float(m.group(3)),
            "syst_error": float(m.group(4)),
        }
    return None


def parse_correlation_matrix(lines, start_idx):
    """
    Parse a correlation matrix block starting after the 'cor. source:' line.
    Returns (matrix_dict, obs_names, next_line_index).
    matrix_dict maps (obs_i, obs_j) -> float.
    """
    idx = start_idx
    # skip blank lines
    while idx < len(lines) and lines[idx].strip() == "":
        idx += 1
    if idx >= len(lines):
        return {}, [], idx

    # header row with column indices
    header = lines[idx].strip().split()
    col_indices = [int(x) for x in header]
    idx += 1

    obs_names = []
    matrix = []
    while idx < len(lines):
        line = lines[idx].strip()
        if line == "" or not line:
            break
        # line like: obs_name   0  1.000  0.180 ...
        parts = line.split()
        # find where the row index is (an integer)
        # format: obs_name  row_idx  val val val ...
        obs_name = parts[0]
        row_idx = int(parts[1])
        values = []
        for v in parts[2:]:
            if v == "-":
                values.append(0.0)
            else:
                values.append(float(v))
        obs_names.append(obs_name)
        matrix.append(values)
        idx += 1

    return matrix, obs_names, idx


def parse_detailed_pdf(lines, start_idx):
    """
    Parse a single detailed PDF block.
    Returns (pdf_dict, next_line_index).
    """
    pdf = {}
    idx = start_idx

    # First line: " N. PDF: <name> (UID<n>)"
    line = lines[idx].strip()
    m = re.match(r"(\d+)\.\s+PDF:\s+(\S+)\s+\(UID(\d+)\)", line)
    if not m:
        return None, idx + 1
    pdf["index"] = int(m.group(1))
    pdf["pdf_name"] = m.group(2)
    pdf["uid"] = int(m.group(3))
    idx += 1

    # Parse sub-sections
    pdf["observables"] = {
        "n_obs": 0,
        "values_source": "",
        "errors_source": "",
        "values": [],
    }
    pdf["correlations"] = {}
    pdf["parameters"] = {"n_par": 0, "names": []}
    pdf["relations"] = []

    while idx < len(lines):
        line = lines[idx]
        stripped = line.strip()

        # Check if we've reached the next PDF entry or end
        if re.match(r"\s*\d+\.\s+PDF:", stripped):
            break

        # observables section
        if stripped == "observables:":
            idx += 1
            while idx < len(lines):
                l = lines[idx].strip()
                if (
                    l == ""
                    or l.startswith("correlation")
                    or l == "parameters:"
                    or l == "relations:"
                ):
                    break
                m_nobs = re.match(r"nObs\s*=\s*(\d+)", l)
                if m_nobs:
                    pdf["observables"]["n_obs"] = int(m_nobs.group(1))
                    idx += 1
                    continue
                m_valfrom = re.match(r"values from:\s*(.*)", l)
                if m_valfrom:
                    pdf["observables"]["values_source"] = m_valfrom.group(1).strip()
                    idx += 1
                    continue
                m_errfrom = re.match(r"errors from:\s*(.*)", l)
                if m_errfrom:
                    pdf["observables"]["errors_source"] = m_errfrom.group(1).strip()
                    idx += 1
                    continue
                obs = parse_observable_line(lines[idx])
                if obs:
                    pdf["observables"]["values"].append(obs)
                idx += 1
            continue

        # correlation sections
        m_cor = re.match(r"correlation \((stat\+syst|stat|syst)\):", stripped)
        if m_cor:
            cor_type = m_cor.group(1).replace("+", "_plus_")
            idx += 1
            # next line: "cor. source: ..."
            while idx < len(lines) and lines[idx].strip() == "":
                idx += 1
            cor_source = ""
            if idx < len(lines):
                m_src = re.match(r"\s*cor\. source:\s*(.*)", lines[idx])
                if m_src:
                    cor_source = m_src.group(1).strip()
                    idx += 1
            matrix, obs_names, idx = parse_correlation_matrix(lines, idx)
            pdf["correlations"][cor_type] = {
                "source": cor_source,
                "observable_names": obs_names,
                "matrix": matrix,
            }
            continue

        # parameters section
        if stripped == "parameters:":
            idx += 1
            par_names = []
            while idx < len(lines):
                l = lines[idx].strip()
                if (
                    l == ""
                    or l == "relations:"
                    or l.startswith("correlation")
                    or re.match(r"\d+\.\s+PDF:", l)
                ):
                    break
                m_npar = re.match(r"nPar\s*=\s*(\d+)", l)
                if m_npar:
                    pdf["parameters"]["n_par"] = int(m_npar.group(1))
                    idx += 1
                    continue
                # parameter names are comma-separated, possibly across lines
                names = [n.strip() for n in l.split(",") if n.strip()]
                par_names.extend(names)
                idx += 1
            pdf["parameters"]["names"] = par_names
            continue

        # relations section
        if stripped == "relations:":
            idx += 1
            while idx < len(lines):
                l = lines[idx].strip()
                if (
                    l == ""
                    or l.startswith("correlation")
                    or l == "parameters:"
                    or l == "observables:"
                    or re.match(r"\d+\.\s+PDF:", l)
                ):
                    break
                m_rel = re.match(r"(\S+)\s+=\s+(.*)", l)
                if m_rel:
                    pdf["relations"].append(
                        {
                            "observable": m_rel.group(1),
                            "expression": m_rel.group(2).strip(),
                        }
                    )
                idx += 1
            continue

        idx += 1

    return pdf, idx


def strip_ansi(text):
    """Remove ANSI escape sequences from text."""
    return re.sub(r"\x1b\[[0-9;]*m", "", text)


def lookup_combination_name(source_path, combination_id):
    """Parse a gammacombo main .cpp file to find the short name for a combination ID.

    Matches patterns like:
        gc.newCombiner(50, "hflav_pdg2025", ...)
        gc.cloneCombiner(50, 41, "hflav_pdg2025", ...)
    """
    with open(source_path) as f:
        source = f.read()
    # newCombiner(ID, "name", ...)
    m = re.search(rf'gc\.newCombiner\(\s*{combination_id}\s*,\s*"([^"]+)"', source)
    if m:
        return m.group(1)
    # cloneCombiner(ID, sourceID, "name", ...)
    m = re.search(
        rf'gc\.cloneCombiner\(\s*{combination_id}\s*,\s*\d+\s*,\s*"([^"]+)"',
        source,
    )
    if m:
        return m.group(1)
    return None


def parse_info_output(text):
    """Parse the full --info output text into a structured dict."""
    text = strip_ansi(text)
    lines = text.split("\n")
    result = {
        "software_version": "",
        "executable": "",
        "combination_id": None,
        "combination_name": "",
        "combination_title": "",
        "measurements_summary": [],
        "input_observables": [],
        "free_parameters": [],
        "detailed_measurements": [],
    }

    idx = 0
    while idx < len(lines):
        line = lines[idx]
        stripped = line.strip()

        # Software version
        m_ver = re.match(r"GammaCombo\s+([\d.]+)", stripped)
        if m_ver:
            result["software_version"] = m_ver.group(1)
            idx += 1
            continue

        # Combiner title
        m_comb = re.match(r"Combiner Configuration:\s*(.*)", stripped)
        if m_comb:
            result["combination_title"] = m_comb.group(1).strip()
            idx += 1
            continue

        # Measurement summary lines like: " 1. [measurement 101] HFLAV    Bd Lifetime"
        m_meas = re.match(r"(\d+)\.\s+\[measurement\s+(\d+)\]\s+(.*)", stripped)
        if m_meas:
            result["measurements_summary"].append(
                {
                    "index": int(m_meas.group(1)),
                    "measurement_id": int(m_meas.group(2)),
                    "description": m_meas.group(3).strip(),
                }
            )
            idx += 1
            continue

        # Input observables line
        m_obs = re.match(r"(\d+)\s+input observables:\s*\(\s*(.*)", stripped)
        if m_obs:
            obs_text = m_obs.group(2)
            # may span multiple lines until closing )
            while ")" not in obs_text and idx + 1 < len(lines):
                idx += 1
                obs_text += " " + lines[idx].strip()
            obs_text = obs_text.rstrip(")")
            result["input_observables"] = [
                o.strip() for o in obs_text.split(",") if o.strip()
            ]
            idx += 1
            continue

        # Free parameters line
        m_par = re.match(r"(\d+)\s+free parameters:\s*\(\s*(.*)", stripped)
        if m_par:
            par_text = m_par.group(2)
            while ")" not in par_text and idx + 1 < len(lines):
                idx += 1
                par_text += " " + lines[idx].strip()
            par_text = par_text.rstrip(")")
            result["free_parameters"] = [
                p.strip() for p in par_text.split(",") if p.strip()
            ]
            idx += 1
            continue

        # Detailed Configuration section
        if stripped == "Detailed Configuration:":
            idx += 1
            # skip separator
            while idx < len(lines) and lines[idx].strip().startswith("==="):
                idx += 1
            # now parse individual PDF blocks
            while idx < len(lines):
                # skip blank lines
                if lines[idx].strip() == "":
                    idx += 1
                    continue
                if re.match(r"\s*\d+\.\s+PDF:", lines[idx].strip()):
                    pdf, idx = parse_detailed_pdf(lines, idx)
                    if pdf:
                        result["detailed_measurements"].append(pdf)
                else:
                    idx += 1
            continue

        idx += 1

    return result


def main():
    parser = argparse.ArgumentParser(
        description="Convert gammacombo --info output to JSON"
    )
    parser.add_argument("-o", "--output", required=True, help="Output JSON file path")
    parser.add_argument(
        "-i",
        "--input",
        default=None,
        help="Input file (default: stdin)",
    )
    parser.add_argument(
        "-c",
        "--combination-id",
        type=int,
        default=None,
        help="Combination ID (e.g. 50)",
    )
    parser.add_argument(
        "--source",
        default=None,
        help="Path to main .cpp file to look up combination short name",
    )
    parser.add_argument(
        "--executable",
        default=None,
        help="Executable name (e.g. bin/alpha_wa)",
    )
    args = parser.parse_args()

    if args.input:
        with open(args.input) as f:
            text = f.read()
    else:
        text = sys.stdin.read()

    result = parse_info_output(text)

    if args.executable:
        result["executable"] = args.executable

    if args.combination_id is not None:
        result["combination_id"] = args.combination_id

    if args.source and args.combination_id is not None:
        name = lookup_combination_name(args.source, args.combination_id)
        if name:
            result["combination_name"] = name
        else:
            print(
                f"WARNING: could not find combination {args.combination_id} "
                f"in {args.source}",
                file=sys.stderr,
            )

    with open(args.output, "w") as f:
        json.dump(result, f, indent=2)

    n_meas = len(result["detailed_measurements"])
    n_obs = len(result["input_observables"])
    n_par = len(result["free_parameters"])
    print(
        f"Wrote {args.output}: {n_meas} measurements, "
        f"{n_obs} observables, {n_par} parameters",
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
