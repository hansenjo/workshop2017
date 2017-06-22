# Source this script to set up analysis in this directory
# Assumes ROOT and ANALYZER are set up

if [ "x${BASH_ARGV[0]}" = "x" ]; then
    THIS="."
else
    THIS=$(dirname ${BASH_ARGV[0]})
    if [ "${THIS:0:1}" != "/" ]; then
	[ "${THIS}" = "." ] && unset THIS
	THIS=${THIS%/}
	THIS=$PWD/$THIS
	THIS=${THIS%/}
    fi
fi

export DB_DIR=${THIS}/DB
export DATA_DIR=${THIS}/data
export OUT_DIR=${THIS}/rootfiles

