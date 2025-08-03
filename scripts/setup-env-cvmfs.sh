#!/bin/bash

# Run `setup-env-cvmfs.sh -h` for usage help


if grep -q "Red Hat Enterprise Linux 9\|AlmaLinux 9" /etc/os-release; then
  OS="RHEL9/Alma9"
  LCG_VERSION_DEFAULT=LCG_105
  BINARY_TAG_GCC_DEFAULT=x86_64-el9-gcc13-opt
  BINARY_TAG_CLANG_DEFAULT=x86_64-el9-clang16-opt
elif grep -q "Red Hat Enterprise Linux 8" /etc/os-release; then
  OS="Centos8"
  BINARY_TAG_CLANG_DEFAULT=NONE
  LCG_VERSION_DEFAULT=LCG_101
  BINARY_TAG_GCC_DEFAULT=x86_64-centos8-gcc11-opt
else
  echo "The operating system of this computer is not supported by this script."
  return 1
fi

help() {
  echo "Setup a (LCG, compiler) view on a system with CVMFS installed for a consistent development environment"
  echo "In case no LCG version or binary tag are specified, some defaults (with GCC) will be chosen for you."
  echo
  echo "Syntax: setup-env.sh [-h|l|b|c]"
  echo "options:"
  echo "h  Print this help."
  echo "l  LCG version to use (optional, default is '${LCG_VERSION_DEFAULT}')."
  echo "b  Binary tag to use (optional, default is '${BINARY_TAG_GCC_DEFAULT}' for gcc"
  echo "                                           '${BINARY_TAG_CLANG_DEFAULT}' for clang)."
  echo "c  Use Clang rather than GCC as compiler (used only if 'b' is not set)."
  echo
  echo "Example: 'source setup-env.sh -l LCG_105 -b x86_64-el9-clang16-opt'"
  echo
}

# Read the options
clang=false
lcg_setup=false
binary_setup=false
use_custom_lcg_view=false
optstring=":hl:b:cr"
export OPTIND=1
while getopts ${optstring} opt; do
  case ${opt} in
    h) # Display help
      help
      return ;;
    c) # Setup Clang as compiler
      clang=true ;;
    l)
      lcg_setup=true
      export LCG_VERSION=${OPTARG} ;;
    b)
      binary_setup=true
      export BINARY_TAG=${OPTARG} ;;
    ?)
      echo "Unsupported option: ${OPTARG}"
      return ;;
  esac
done
if [[ $binary_setup = "true" && $clang = "true" ]] ; then
  echo "WARNING: The option -c is ignored as the binary tag ${BINARY_TAG} was set explicitly"
fi

# Setup the LCG version and binary tag ---------------------------------------------------------------------------------

echo "Will build GammaCombo with the following configuration:"
echo "  OS:           ${OS}"
if [ ${lcg_setup} = false ]; then
  export LCG_VERSION=${LCG_VERSION_DEFAULT}
fi
if [ ${binary_setup} = false ]; then
  if ${clang} ; then
    export BINARY_TAG=${BINARY_TAG_CLANG_DEFAULT}
  else
    export BINARY_TAG=${BINARY_TAG_GCC_DEFAULT}
  fi
  if [[ ${BINARY_TAG} == "NONE" ]]; then
    echo "ERROR: default BINARY_TAG does not exist for ${OS}. Exit..."
    return 1
  fi
fi
echo "  LCG version:  ${LCG_VERSION}"
echo "  Binary tag:   ${BINARY_TAG}"
source /cvmfs/sft.cern.ch/lcg/views/setupViews.sh $LCG_VERSION $BINARY_TAG
