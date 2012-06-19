PAIR_UTILITIES
==============

Utilities to process pre-calculated pairs.  This includes translating them
from a FITS format (ala Yue Shen), and binning them for results. There are
several additional testing utilities as well.

The code is in three parts:
 1. C code "libraries" (generic functions for easy tool creation)
 2. compiled utilities (binaries that do something)
 3. post-processing scripts

Comments, suggestions, tweaks, and edits are welcome.


GENERAL USE
-----------

Assuming one has a FITS file containing pairs of interest,
there are three things to do:

 1. translate FITS to the desired PAIRS format
    e.g. see translate_pairs_proj

 2. create WEIGHT files

 3. do something with the PAIRS file
    e.g. see bin_pairs_proj

 4. create statistic from counts
    e.g. see calc_cross_corr.rb

 5. [optional] calculate covariance matrix
    e.g. see `calc_jack_covar.rb`

There is a example bash shell script that runs steps 3-5 which
is included, see: `scripts/run_bin_proj.sh`

NOTE: some filtering and assumed bins are HARD CODED in the source.


C LIBRARIES
-----------

There are three current code files that define functions and are
library-like. These do most of the heavy lifting for the above utilities,
and are intended to be able to easily create new utilities.

 * `lib/bins.h`

    abstracted creation and utilities for binning, should be n-dimensional

 * `lib/pairs.h`

    definition, reading, and writing of the PAIRs format

 *  `lib/weight_set.h`

    a quick set of functions to define a "weight set" and read such
    from an ASCII space delimited file


C UTILITIES
-----------

There are a number of useful binary utilities intended just for use.

 * `translate_pairs_proj  FITS_FILE PAIRS_FILE `

    convert FITS pair file into truncated (slightly filtered)
    PAIRS file in PAIR_PROJ format (i.e. rp,pi and IDs only).
    Filtering is HARD CODED, edit as you see fit!


 * `bin_pairs_proj  OUT_BASE  WEIGHT1  WEIGHT2  PAIRS* `

    read a number of PAIRS files and two weight files associated
    with ID1 (first) and ID2 (second). Produce a binned count
    (including normalized version) as output with OUT_BASE as the
    start of the file (extension added automatically).

    The weight files have to be at least 1 column:
	1. ID (integer)
	2. WEIGHT (float weight value)

    Additional columns are skipped (although a 3rd is read). If
    the 2nd column (WEIGHT) is neglected, a weight of '1' is
    assumed.  Any ID not listed is assumed to have a weight of
    '0' and *not* counted in pair file.

    The "normalized" counts weight each pair by the product of
    the weights (count += w1 * w2), and normalize by the total
    weight represented in the weight files.

    NOTE: The binning is hard coded, edit source to change it!


 * `bin_pairs_proj_jack  OUT_BASE WEIGHT1 WEIGHT2  PAIRS* `

    Similar utility to "bin_pairs_proj" but now does this for
    a number of jackknife samples.  Produces one binned output file
    per jackknife sample using OUT_BASE as the start of the file.

    The 3rd column of the WEIGHT file defines which ID belongs
    to which jackknife sample.  The number of jackknife samples
    is auto determined based on these (zero based) jackknife IDs.

    NOTE: The binning is hard coded, edit source to change it!


POST-PROCESSING SCRIPTS
-----------------------

These are simple scripts (in ruby, a language similar to python / perl)
for various types of post-processing.  For example, combine counts
into a statistic, such as xi(r), or calculate a jackknife covariance
matrix. These should work for any recently modern version of ruby
(tested on MRI version 1.8.7).

 * `scripts/calc_cross_corr.rb`

    take binned output from bin_pairs_XXX and compute
    statistic (QG / QR - 1).  It can also integrate the
    output if in rp-pi space, and so requested.

 *  `calc_jack_covar.rb`

    calculate jackknife covariance matrix from a number of samples.

Typically, one runs these without an argument to get a usage statement.


---

Cameron McBride 
cameron.mcbride@gmail.com

June 2012
