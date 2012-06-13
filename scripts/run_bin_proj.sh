#!/bin/bash

## This is an shell script meant to display the full processing.
## There are several conventions for filenames here
## Two initial steps are required:
## 1. create PAIRS files
## 2. create WEIGHTS files

bdir="."             # binary directory
sdir="./scripts"     # post-processing scripts directory
wdir="./qso_galaxy"  # directory containing weights
pdir="./pairs"       # directory containing pairs files
rdir="./results"     # diectory to put middle results
odir="."             # master output directory

bpp="$bdir/bin_pairs_proj"
bppj="$bdir/bin_pairs_proj_jack"
ccc="$sdir/calc_cross_corr.rb"
cjc="$sdir/calc_jack_covar.rb"

$bpp $rdir/proj.QG $wdir/qso.weight $wdir/gal.weight $pdir/QG_proj.pairs
$bpp $rdir/proj.QR $wdir/qso.weight $wdir/ran.weight $pdir/QR_proj.pairs
qg=$rdir/proj.QG.full.dat
qr=$rdir/proj.QR.full.dat
xi=$rdir/proj_full.xi.dat
wp=$odir/proj_full.wp.dat
$ccc $qg $qr $xi $wp

# for nj in 10 25 50 75 100
for nj in 25
do
    js="${nj}j"
    $bppj $rdir/proj.QG $wdir/qso_$js.weight $wdir/gal_$js.weight $pdir/QG_proj.pairs
    $bppj $rdir/proj.QR $wdir/qso_$js.weight $wdir/ran_$js.weight $pdir/QR_proj.pairs
    for ij in $(gseq 0 $(($nj - 1)) )
    do
	id=$(printf "%03d" $ij)
	qg=$rdir/proj.QG.${js}$id.dat
	qr=$rdir/proj.QR.${js}$id.dat
	xi=$rdir/proj_${js}$id.xi.dat
	wp=$rdir/proj_${js}$id.wp.dat
	$ccc  $qg $qr $xi $wp
    done
    $cjc $odir/proj_${js}.wp $rdir/proj_${js}*.wp.dat
done
