#/bin/bash

export LANG=C
export LC_ALL=C

mkdir -p mnt
./tagfs $PWD/images $PWD/mnt
sleep 1
ln mnt/ipb.jpeg mnt/ecole/ipb.jpeg
mv mnt/ecole/ipb.jpeg mnt/ecoleSup/
ln mnt/marmotte.jpeg mnt/gentil/doux/calme/marmotte.jpeg
#rm mnt/mechant/rabbit.jpeg
#rm mnt/monty/animal/rabbit.jpeg
#mv mnt/mechant/coyote.jpeg mnt/gentil/cartoon/coyote.jpeg

fusermount -u mnt

(cat <<EOF
[bilbo.jpeg]
cartoon
gentil
hobbit

[coyote.jpeg]
animal
mechant

[gru.jpeg]
gentil

[ipb.jpeg]
ecoleSup

[marmotte.jpeg]
animal
gentil
doux
calme

[rabbit.jpeg]
animal
monty

EOF
) > check_$$

diff $PWD/images/.tags check_$$ > diff_$$
if test -s diff_$$
then
    echo "Test ... Failure"
    cat diff_$$
else
    echo "Test ... Pass"
fi
rm -f check_$$ diff_$$

