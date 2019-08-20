echo "#####################################################"
echo "###   Please fun the following commands in order  ###"
echo "###   (note that the second one will load it's    ###"
echo "###    own environment so you cannot just source  ###"
echo "###    this script in bash)                       ###"
echo "#####################################################"
echo '    LbLogin -c x86_64-slc6-gcc49-opt'
echo '    lb-run DaVinci/v42r8p3 bash'
echo '    source $(dirname $(dirname `which gcc`))/setup.sh'
