import os
import glob
import math
import ROOT
from ROOT import *
import sys

from optparse import OptionParser

parser = OptionParser()

parser.add_option('--cut', metavar='F', type='string', action='store',
                  dest='cut',
                  help='')

parser.add_option('--var', metavar='F', type='string', action='store',
                  dest='var',
                  help='')

parser.add_option('--Min', metavar='F', type='float', action='store',
                  dest='Min',
                  help='')

parser.add_option('--Max', metavar='F', type='float', action='store',
                  dest='Max',
                  help='')

parser.add_option('--name', metavar='F', type='string', action='store',
	    	  default = "blahblahblah",
                  dest='name',
                  help='')

parser.add_option('--log', metavar='F', type='string', action='store',
                  default='no',
                  dest='log',
                  help='')

parser.add_option('--scale', metavar='F', type='float', action='store',
                  default='1.0',
                  dest='scale',
                  help='')


parser.add_option('--bin', metavar='F', type='int', action='store',
                  default=100,
                  dest='bin',
                  help='')

parser.add_option('--file', metavar='F', type='string', action='store',
                  default='no',
                  dest='fi',
                  help='')


(options, args) = parser.parse_args()

scale = options.scale
cut = options.cut
var = options.var
x = options.Min
y = options.Max
log = options.log
bin = options.bin
fi = options.fi
name = options.name

f = ROOT.TFile( options.name + ".root", "recreate" )
f.cd()

chain = ROOT.TChain("treeVars")
chain.Add(fi)
newhist = ROOT.TH1F(name, name, bin, x, y)	
chain.Draw(var+">>"+name,""+ cut, "goff")
newhist.Scale(scale)
newhist.SetLineColor(ROOT.kBlue)
newhist.SetFillColor(0)
newhist.SetLineWidth(2)
newhist.SetLineStyle(2)	
newhist.SetStats(0)
f.Write()


c = TCanvas()
c.cd()
newhist.SetTitle("")
newhist.GetXaxis().SetTitle(var + "  w/  " + cut)
if cut == "":
	newhist.GetXaxis().SetTitle(var)
newhist.GetYaxis().SetTitle("events")

if log == "yes":
	c.SetLogy()

newhist.Draw()
c.SaveAs(name + ".pdf")



