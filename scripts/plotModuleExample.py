#!/usr/bin/env python
import ROOT as r

def setTextBoxAttributes(text, color, font):
  text.SetTextColor(color)
  text.SetTextAlign(13)
  text.SetTextSize(0.04)
  text.SetTextFont(font)

def dress(canv, colors):

  # need this to keep the objects alive

  objs = []

  vub_inc = r.TLatex(0.16,0.655,"V_{ub} inclusive")
  vub_inc.SetNDC()
  vub_inc.Draw("same")
  objs.append(vub_inc)

  vcb_inc = r.TLatex(0.68,0.72,"V_{cb} inclusive")
  vcb_inc.SetNDC()
  vcb_inc.SetTextAngle(90)
  vcb_inc.Draw("same")
  objs.append(vcb_inc)

  vub_excl = r.TLatex(0.16,0.455,"V_{ub} exclusive")
  vub_excl.SetNDC()
  vub_excl.Draw("same")
  objs.append(vub_excl)

  vcb_excl = r.TLatex(0.45,0.72,"V_{cb} exclusive")
  vcb_excl.SetNDC()
  vcb_excl.SetTextAngle(90)
  vcb_excl.Draw("same")
  objs.append(vcb_excl)

  vub_vcb_lhcb = r.TLatex(0.17,0.29,"V_{ub}/V_{cb} LHCb")
  vub_vcb_lhcb.SetNDC()
  vub_vcb_lhcb.SetTextAngle(8)
  vub_vcb_lhcb.Draw("same")
  objs.append(vub_vcb_lhcb)

  indirect = r.TLatex(0.17,0.38,"Indirect (CKM fitter)")
  indirect.SetNDC()
  indirect.SetTextAngle(9)
  indirect.Draw("same")
  objs.append(indirect)

  comb_inc = r.TLatex(0.66,0.61,"Comb. incl.")
  comb_inc.SetNDC()
  comb_inc.Draw("same")
  objs.append(comb_inc)

  comb_excl = r.TLatex(0.43,0.40,"Comb. excl.")
  comb_excl.SetNDC()
  comb_excl.SetTextAngle(0)
  comb_excl.Draw("same")
  objs.append(comb_excl)

  group1 = r.TLatex(0.18,0.85,"#splitline{PDG 2014 +}{CKM fitter +}")
  group1.SetNDC()
  group1.SetTextFont(132)
  group1.SetTextSize(0.05)
  group1.Draw("same")
  objs.append(group1)

  group2 = r.TLatex(0.18,0.75,"#splitline{#Lambda_{b}#rightarrowp#mu#nu (LHCb)}{}")
  group2.SetNDC()
  group2.SetTextFont(132)
  group2.SetTextSize(0.05)
  group2.Draw("same")
  objs.append(group2)

  canv.Update()
  canv.Modified()

  return objs

