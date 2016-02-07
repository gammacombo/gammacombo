#!/usr/bin/env python

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-t","--title",default="",help="Title")
parser.add_option("-w","--width",default=300,help="Plot width")
parser.add_option("-p","--plotsPerLine",type="int",default=-1,help="Plots to display per line")
parser.add_option("-c","--colorScheme",default="maroon",help="html color (as string) for borders etc.")
parser.add_option("-u","--upload",default=None, help='Upload location on afs web server')
parser.add_option("-l","--lxplus",default=False, action="store_true", help='If already running on lxplus')
parser.add_option("-T","--date", default=None,help="Filter only files modified after this date - format dd/mm/yyyy. Default=%default")
parser.add_option("-r","--regex",default=None, help='Each plot name must match this regex')
parser.add_option("-d","--dir",default="plots",help='Directory with plots in')
parser.add_option("-n","--newLoc",default=None, help='Copy plots and html file into a seperate location')
(opts,args) = parser.parse_args()

import os
import re
import time
import datetime
regex=None
if opts.regex:
	regex = re.compile(opts.regex)

def gatherDirectories(path):

  directories = []
  for root, dirs, files in os.walk(path):
    if root != path: # depth 1
      break
    for directory in dirs:
      directories.append( os.path.join(root,directory) )

  return directories

def gatherFiles(path):

  ret_files = []
  for root, dirs, files in os.walk(path):
    if root != path:
      break
    for fil in files:
      # check regex
      if regex:
        if not regex.match(fil):
          continue
      # check modified date
      if opts.date:
        d = datetime.datetime( int(opts.date.split('/')[2]), int(opts.date.split('/')[1]), int(opts.date.split('/')[0]) )
        dsecs = (d-datetime.datetime(1970,1,1)).total_seconds()
        if os.path.getmtime(os.path.join(root,fil)) < dsecs:
          continue
      # add to list
      ret_files.append( os.path.join(root, fil) )

  return ret_files

def gatherPlots( directory ):
  plots = {}
  plotTypes = gatherDirectories( directory )
  for plotType in plotTypes:
    plotExt = os.path.basename( plotType )
    plots[plotExt] = gatherFiles( plotType )
  return plots

def insertJavaDropDownScript(f):
  f.write('<script language=\"JavaScript\" type=\"text/javascript\">\n')
  f.write('if (document.getElementById) {\n')
  f.write('document.writeln(\'<style type=\"text/css\"><!--\')\n')
  f.write('document.writeln(\'.texter {display:none} @media print {.texter {display:block;}}\')\n')
  f.write('document.writeln(\'</style>\') }\n')
  f.write('function openClose(theID) {\n')
  f.write('if (document.getElementById(theID).style.display == \"block\") { document.getElementById(theID).style.display = \"none\" }\n')
  f.write('else { document.getElementById(theID).style.display = \"block\" } }\n')
  f.write('</script>\n')

def writeHtml( location, title, links, plots, isHome=False ):
  html = open( os.path.join( location, 'index.html' ), 'w' )
  insertJavaDropDownScript( html )

  # title
  if opts.title != "" and title != opts.title:
    title = opts.title + " - " + title
  html.write('<font size=\"5\"> <u> '+title+' </u> </font> <br>\n')
  html.write('<script language=\"Javascript\"> document.write(\"Last modified: \" + document.lastModified + \" (UTC)\"); </script> <br>\n')
  html.write('<br>\n')

  # check for pngs
  if 'png' not in plots.keys() and not isHome:
    html.write('No png files found\n')
    html.close()
    return

  # links
  #if isHome:
    #html.write('<b><u><font color=\"blue\"> Contents: </font></u></b> <br>\n')
    #for link in links:
      #link_name = os.path.basename(link)
      #html.write('&#160; &#160; &#8627; <a href=\"'+link_name+'/index.html\">'+link_name+'</a> <br> \n')
    #return
  #else:
    #html.write('<div onClick=\"openClose(\'links\')\" style=\"cursor:hand; cursor:pointer\"><b><u><font color=\"blue\"> Other plots: </font></u></b> (click to expand) </div> \n')
    #html.write('<div id=\"links\" class=\"texter\"> \n')
    #html.write('&#160; &#160; <a href=\"../index.html\">Home</a> <br> \n')
    #for link in links:
      #link_name = os.path.basename(link)
      #html.write('&#160; &#160; &#8627; <a href=\"../'+link_name+'/index.html\">'+link_name+'</a> <br> \n')
    #html.write('</div>\n')
    #html.write('<br>\n')

  # contents
  html.write('<div onClick=\"openClose(\'contents\')\" style=\"cursor:hand; cursor:pointer\"><b><u><font color=\"blue\"> Contents: </font></u></b> (click to expand) </div> \n')
  html.write('<div id=\"contents\" class=\"texter\"> \n')
  for png in plots['png']:
    png_path = os.path.relpath( png, location )
    basename = os.path.basename(os.path.splitext(png_path)[0])
    if ( os.path.splitext(png_path)[1] != '.png' ): continue
    html.write('&#160; &#160; <a href=\"#'+basename+'\">'+basename+'</a> <br>\n')
  html.write('</div>\n')
  html.write('<br>\n')

  print 'The following thumbnails will be used:'
  # plots
  for i, png in enumerate(plots['png']):
    png_path = os.path.relpath( png, location )
    basename = os.path.basename(os.path.splitext(png_path)[0])

    # if not a .png file then skip it
    if ( os.path.splitext(png_path)[1] != '.png' ): continue

    # if there's a pdf equivalent use this as the link path for the thumbnail png
    link_path = png_path
    if 'pdf' in plots.keys():
      pdf_path = png_path.replace('png/','pdf/')
      pdf_path = pdf_path.replace('.png','.pdf')
      if os.path.join( location, pdf_path ) in plots['pdf']:
        link_path = pdf_path

    html.write('<div style=\"display:inline-block;border-style:groove;border-width:5px;color:%s;width:%s;word-break:break-all;word-wrap:break-all;\">\n'%(opts.colorScheme,opts.width))
    html.write('\t<center>\n')
    html.write('\t\t<a id=\"'+basename+'\" href='+link_path+'><img width=\"'+str(opts.width)+'\" src=\"'+png_path+'\"></a><br>\n')
    html.write('\t\t<b>'+basename+'</b><br>\n')
    for ext in plots.keys():
      ext_path = png_path.replace('png/',ext+'/')
      ext_path = ext_path.replace('.png','.'+ext)
      if os.path.join( location, ext_path ) in plots[ext]:
        html.write(' &#160; <a href=\"'+ext_path+'\">'+ext+'</a>')
    html.write('<br>\n')
    html.write('\t</center>\n')
    html.write('</div>\n')

    if opts.plotsPerLine>1 and i%opts.plotsPerLine==0: html.write('<br>\n')

    print '\t', png_path.split('png/')[1]

  html.close()

  # if new location then copy everything to this new place
  if opts.newLoc:
    os.system('mkdir -p %s'%opts.newLoc)
    os.system('cp %s %s/'%(html.name, opts.newLoc))

    for ext, files in plots.iteritems():
      if len(files)>0:
        os.system('mkdir -p %s/%s'%(opts.newLoc,ext))
        for f in files:
          os.system('cp %s %s/%s/'%(f,opts.newLoc,ext))

  # print message
  outloc = opts.newLoc if opts.newLoc else opts.dir
  print 'Page written to: \n\t%s/index.html'%outloc

# __ main __

plots = gatherPlots( opts.dir )
writeHtml( opts.dir, opts.title, [opts.dir], plots )

if opts.upload:

  print 'Will upload to the following afs location: \n\t%s '%opts.upload

  current_loc = opts.newLoc if opts.newLoc else opts.dir
  if opts.lxplus:
    exec_line = 'cp -r %s/* %s/'%(current_loc,opts.upload)
  else:
    uname = raw_input('Enter username@lxplus.cern.ch\n')
    exec_line = 'scp -r %s/* %s@lxplus.cern.ch:%s/'%(current_loc,uname,opts.upload)
  print exec_line

  os.system( exec_line )


