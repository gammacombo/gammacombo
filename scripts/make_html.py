#!/usr/bin/env python

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-t","--title",default="",help="Title")
parser.add_option("-w","--width",default=300,help="Plot width")
parser.add_option("-p","--plotsPerLine",type="int",default=-1,help="Plots to display per line")
parser.add_option("-c","--colorScheme",default="maroon",help="html color (as string) for borders etc.")
parser.add_option("-u","--upload",default=None, help='Upload location on afs web server')
parser.add_option("-r","--regex",default=None, help='Each plot name must match this regex')
(opts,args) = parser.parse_args()

import os
import re
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
			if regex:
				if regex.match(fil):
					ret_files.append( os.path.join(root, fil) )
			else:
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
  if opts.title != "":
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

  # plots
  for i, png in enumerate(plots['png']):
    png_path = os.path.relpath( png, location )
    basename = os.path.basename(os.path.splitext(png_path)[0])

    # if not a .png file then skip it
    if ( os.path.splitext(png_path)[1] != '.png' ): continue

    # see if there's a pdf equivalent which can be linked to
    link_path = png_path
    if 'pdf' in plots.keys():
      pdf_path = png_path.replace('png/','pdf/')
      pdf_path = pdf_path.replace('.png','.pdf')
      if os.path.join( location, pdf_path ) in plots['pdf']:
        link_path = pdf_path

    html.write('<div style=\"display:inline-block;border-style:groove;border-width:5px;color:%s\">\n'%opts.colorScheme)
    html.write('\t<center>\n')
    html.write('\t\t<a id=\"'+basename+'\" href='+link_path+'><img width=\"'+str(opts.width)+'\" src=\"'+png_path+'\"></a><br>\n')
    html.write('\t\t<b>'+basename+'</b><br>\n')
    #html.write('\t\t<a href=\"'+png_path+'\">png</a>')
    for ext in plots.keys():
      ext_path = png_path.replace('png/',ext+'/')
      ext_path = ext_path.replace('.png','.'+ext)
      if os.path.join( location, ext_path ) in plots[ext]:
        html.write(' &#160; <a href=\"'+ext_path+'\">'+ext+'</a>')
    html.write('<br>\n')
    html.write('\t</center>\n')
    html.write('</div>\n')

    if opts.plotsPerLine>1 and i%opts.plotsPerLine==0: html.write('<br>\n')

    print '\t \t', png_path

  html.close()


# __ main __

#directories = gatherDirectories('plots')
directories = ['plots']

# home page
#writeHtml( 'plots', 'Home', directories, {}, True )

# subdir pages
for directory in directories:

  name = os.path.basename( directory )
  plots = gatherPlots( directory )

  print name

  writeHtml( directory, name, directories, plots )

if opts.upload:

  print 'Will upload to the following afs location: %s '%opts.upload

  uname = raw_input('Enter username@lxplus.cern.ch\n')

  exec_line = 'scp -r plots/* %s@lxplus.cern.ch:%s/'%(uname,opts.upload)
  print exec_line

  os.system( exec_line )


