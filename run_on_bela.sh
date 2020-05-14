#!/bin/sh

BELADIR="../_bela_run"
SRCDIR="./src"

if [ -d "$BELADIR" ]; then
    rmdir $BELADIR
fi
mkdir -p $BELADIR

BELADIRABS=$(realpath ${BELADIR})

pushd ${SRCDIR} > /dev/null

for f in $(find . -name "*.cpp" -or -name "*.hpp")
do
    # Exclude unit tests - main() interferes with Bela code
    if ! ([[ "$f" =~ 'main.cpp' ]] || [[ "$f" =~ 'py_wrapper' ]] ) ; then
        cp "$f" $BELADIRABS
    else
        echo "Skipping $f"
    fi
done

popd > /dev/null

echo "Copied files to ${BELADIR}."

../scripts/build_project.sh ${BELADIR}
