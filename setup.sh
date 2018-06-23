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

[ -z "$WORKSHOP_DATA" ] && export WORKSHOP_DATA=/data

if [ ! -e ${THIS}/data ]; then
    if [ ! -z "$WORKSHOP_DATA" -a -r ${WORKSHOP_DATA}/raw ]; then
	ln -s ${WORKSHOP_DATA}/raw ${THIS}/data
    else
	echo "$WORKSHOP_DATA/raw not found, creating empty data dir"
	mkdir ${THIS}/data
    fi
fi
if [ ! -e ${THIS}/rootfiles ]; then
    if [ ! -z "$WORKSHOP_DATA" -a -r ${WORKSHOP_DATA}/ROOTfiles ]; then
	ln -s ${WORKSHOP_DATA}/ROOTfiles ${THIS}/rootfiles
    else
	echo "$WORKSHOP_DATA/ROOTfiles not found, creating empty rootfiles dir"
	mkdir ${THIS}/rootfiles
    fi
fi

export DB_DIR=${THIS}/DB
export DATA_DIR=${THIS}/data
export OUT_DIR=${THIS}/rootfiles

