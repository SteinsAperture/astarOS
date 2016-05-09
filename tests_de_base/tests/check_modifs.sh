#/bin/bash

export LANG=C
export LC_ALL=C

mkdir -p mnt
./tagfs $PWD/images $PWD/mnt
sleep 1

#init
TOUS=$(/bin/ls mnt | tr ' ' '\012' | sort)

ln mnt/ipb.jpeg mnt/ecole/ipb.jpeg

TAGIPB1=$(/bin/cat mnt/.ipb.jpeg | tr ' ' '\012' | sort | tr '\012' ' ' | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')

mv mnt/ecole/ipb.jpeg mnt/ecoleSup/ipb.jpeg
TAGIPB2=$(/bin/cat mnt/.ipb.jpeg | tr ' ' '\012' | sort | tr '\012' ' ' | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')


ln mnt/marmotte.jpeg mnt/gentil/doux/calme/marmotte.jpeg
TAGMARMOTTE1=$(/bin/cat mnt/.marmotte.jpeg | tr ' ' '\012' | sort | tr '\012' ' ' | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')


rm mnt/gentil/doux/calme/marmotte.jpeg
TAGMARMOTTE2=$(/bin/cat mnt/.marmotte.jpeg | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' | tr ' ' '\012' | sort | tr '\012' ' ' | tr -d '[[:space:]]')

rm mnt/ecoleSup/ipb.jpeg
TAGIPB3=$(/bin/cat mnt/.ipb.jpeg | tr ' ' '\012' | sort | tr '\012' ' ' | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')

fusermount -u mnt

IMAGES=$(ls $PWD/images)
TAGS=$(grep -v "\[" $PWD/images/.tags | grep -v "^$"|grep -v "#"|sort|uniq)
TAGS_IMAGES=$(echo $TAGS $IMAGES | tr ' ' '\012'| sort)


echo "verification modif"
if test "$TOUS" == "$TAGS_IMAGES"
then
    echo "Test1 ... Pass"
else
    echo "Test1 ... Failure"
    echo "Expected:"
    echo $TAGS_IMAGES
    echo "Obtained:"
    echo $TOUS
fi


TAGIPB1_EXPECTED="ecole"
if test "$TAGIPB1" == "$TAGIPB1_EXPECTED"
then
    echo "Test2 ... Pass"
else
    echo "Test2 ... Failure"
    echo "Expected:"
    echo "'$TAGIPB1_EXPECTED'"
    echo "Obtained:"
    echo "'$TAGIPB1'"
fi

TAGIPB2_EXPECTED="ecoleSup"
if test "$TAGIPB2" == "$TAGIPB2_EXPECTED"
then
    echo "Test3 ... Pass"
else
    echo "Test3 ... Failure"
    echo "Expected:"
    echo "'$TAGIPB2_EXPECTED'"
    echo "Obtained:"
    echo "'$TAGIPB2'"
fi



TAGMARMOTTE1_EXPECTED="animal calme doux gentil"
if test "$TAGMARMOTTE1" == "$TAGMARMOTTE1_EXPECTED"
then
    echo "Test4 ... Pass"
else
    echo "Test4 ... Failure"
    echo "Expected:"
    echo "'$TAGMARMOTTE1_EXPECTED'"
    echo "Obtained:"
    echo "'$TAGMARMOTTE1'"
fi

TAGMARMOTTE2_EXPECTED="animal"
if test "$TAGMARMOTTE2" == "$TAGMARMOTTE2_EXPECTED"
then
    echo "Test5 ... Pass"
else
    echo "Test5 ... Failure"
    echo "Expected:"
    echo "'$TAGMARMOTTE2_EXPECTED'"
    echo "Obtained:"
    echo "'$TAGMARMOTTE2'"
fi

TAGIPB3_EXPECTED=""
if test "$TAGIPB3" == "$TAGIPB3_EXPECTED"
then
    echo "Test6 ... Pass"
else
    echo "Test6 ... Failure"
    echo "Expected:"
    echo "'$TAGIPB3_EXPECTED'"
    echo "Obtained:"
    echo "'$TAGIPB3'"
fi
