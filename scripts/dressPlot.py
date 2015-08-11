#!/usr/bin/env python

import ROOT as r
import os
import sys

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-i","--inputfile",help="The .root file which has the canvas in it which you want to \'dress up\'")
parser.add_option("-o","--outputfile",default="dressed_plot",help="File to write the plots to (no extension and no path)")
parser.add_option("-x","--xaxis",type="str", default=None, help="Set the xaxis range e.g --x 0:10")
parser.add_option("-y","--yaxis",type="str", default=None, help="Set the yaxis range e.g --y 0:10")
parser.add_option("-d","--dressModule",type="str", default=None, help="Dress with this additional module")
parser.add_option("-l","--keepleg",default=False,action="store_true", help="Keep the legend")
parser.add_option("-b","--batch",default=False, action="store_true",help="Run in batch mode")
(opts,args) = parser.parse_args()

if opts.batch:
  r.gROOT.SetBatch()

xaxisrange = None
yaxisrange = None
if opts.xaxis:
  xaxisrange = [ float(opts.xaxis.split(':')[0]), float(opts.xaxis.split(':')[1]) ]
if opts.yaxis:
  yaxisrange = [ float(opts.yaxis.split(':')[0]), float(opts.yaxis.split(':')[1]) ]

def findCanv():
  tf = r.TFile(opts.inputfile)
  canv = None
  for obj in tf.GetListOfKeys():
    theobj = tf.Get(obj.GetName())
    if theobj.ClassName() == 'TCanvas':
      canv = (tf.Get(obj.GetName())).Clone('TheCanvas')

  if not canv:
    sys.exit('No canvas found in file %s'%opts.inputfile)

  tf.Close()
  return canv

def setAxis(canv, axisLabel):

  if axisLabel not in ['t','b','l','r']:
    sys.exit('ERROR - axisLabel must be one of \'t\', \'b\', \'l\' or \'r\'')

  axis = canv.FindObject('axis'+axisLabel)
  if not axis: return

  if axisLabel == 'b':
    if xaxisrange:
      axis.SetX1(xaxisrange[0])
      axis.SetX2(xaxisrange[1])
      axis.SetWmin(xaxisrange[0])
      axis.SetWmax(xaxisrange[1])
    if yaxisrange:
      axis.SetY1(yaxisrange[0])
      axis.SetY2(yaxisrange[0])
  elif axisLabel == 't':
    if xaxisrange:
      axis.SetX1(xaxisrange[0])
      axis.SetX2(xaxisrange[1])
      axis.SetWmin(xaxisrange[0])
      axis.SetWmax(xaxisrange[1])
    if yaxisrange:
      axis.SetY1(yaxisrange[1])
      axis.SetY2(yaxisrange[1])
  elif axisLabel == 'l':
    if xaxisrange:
      axis.SetX1(xaxisrange[0])
      axis.SetX2(xaxisrange[0])
    if yaxisrange:
      axis.SetY1(yaxisrange[0])
      axis.SetY2(yaxisrange[1])
      axis.SetWmin(yaxisrange[0])
      axis.SetWmax(yaxisrange[1])
  elif axisLabel == 'r':
    if xaxisrange:
      axis.SetX1(xaxisrange[1])
      axis.SetX2(xaxisrange[1])
    if yaxisrange:
      axis.SetY1(yaxisrange[0])
      axis.SetY2(yaxisrange[1])
      axis.SetWmin(yaxisrange[0])
      axis.SetWmax(yaxisrange[1])
  else:
    sys.exit('ERROR - axisLabel must be one of \'t\', \'b\', \'l\' or \'r\'')


def setUpHAxis(canv):
  haxis = None
  for prim in canv.GetListOfPrimitives():
    if 'haxes' in prim.GetName():
      haxis = canv.FindObject(prim.GetName())

  if not haxis:
    sys.exit('Axis histogram not found in canvas')

  haxis.SetTitle('')
  haxis.SetDrawOption("AXISG")
  if xaxisrange:
    haxis.GetXaxis().SetRangeUser(xaxisrange[0],xaxisrange[1])
  if yaxisrange:
    haxis.GetYaxis().SetRangeUser(yaxisrange[0],yaxisrange[1])

  setAxis(canv, 'b')
  setAxis(canv, 'b')
  setAxis(canv, 'l')
  setAxis(canv, 'r')

  return haxis

def resetColors(canv):

  line_colors = []
  for prim in canv.GetListOfPrimitives():

    if 'Graph' in prim.GetName():
      if prim.GetFillColor()>1000:
        color = r.gROOT.GetColor(prim.GetFillColor())
        color.SetAlpha(0.7)
      if prim.GetLineColor()>1000:
        line_colors.append(prim.GetLineColor())

  return line_colors

# do stuff
if not os.path.exists(opts.inputfile):
  sys.exit('No file found at: %s'%opts.inputfile)

# find the canvas
canv = findCanv()
canv.Draw()

# sort out the axis histogram
haxis = setUpHAxis(canv)

# get rid of the old legend
leg = canv.FindObject('TPave')
if not opts.keepleg:
	leg.Delete()

# adjust the colors
line_colors = resetColors(canv)

# now do user defined additions
if opts.dressModule:
  path_to_mod = ''
  mod_name = opts.dressModule
  if mod_name.endswith('.py'):
    mod_name = mod_name.strip('.py')

  path_to_mod = os.path.abspath(os.path.dirname(mod_name))
  mod_name = os.path.basename(mod_name)
  sys.path.append(path_to_mod)

  i = __import__(mod_name)
  if not i:
    sys.exit('Could not import custom module %s'%opts.dressModule)

  objs = i.dress(canv,line_colors)

canv.Update()
canv.Modified()
canv.Print('plots/pdf/%s.pdf'%opts.outputfile)
canv.Print('plots/png/%s.png'%opts.outputfile)
canv.Print('plots/eps/%s.eps'%opts.outputfile)
canv.Print('plots/C/%s.C'%opts.outputfile)
canv.SaveAs('plots/root/%s.root'%opts.outputfile)

if not opts.batch:
  raw_input("Ok?")

