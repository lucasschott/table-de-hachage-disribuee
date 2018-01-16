#!/bin/sh

TMP=/tmp/$TEST-$$

fail ()
{
    echo "==> Échec sur '$1'."
    echo "==> Exit"
    killall ./server
    exit 1
}

tu ()
{
    # rappel: "! cmd" => inverse le code de retour de cmd
    ! grep -q "usage: " $TMP
}

cmp_disp ()
{
    grep -q $1 $TMP
}

#####initialisation#####
#pour être sur que des server ne sont pas
#déjà en train de tourner


killall -q ./server




#####test1: test des arguments du server#####
#si les arguments sont invalides, le programme doit
#afficher "usage: ..." et quitter avec une erreur
echo "test1: test des arguments du server"
TEST="test1"


./server x 9000                     2> $TMP >&2 || tu && fail "adresse invalide"
./server localhost x                2> $TMP >&2 || tu && fail "port invalide"
./server                            2> $TMP >&2 || tu && fail "pas d'argument"
./server localhost 9000 locahost    2> $TMP >&2 || tu && fail "3 arguments"
./server localhost 9000 xx          2> $TMP >&2 || tu && fail "2e addr invalide"
./server localhost 9000 localhost x 2> $TMP >&2 || tu && fail "2e port invalide"




#####test2: test des arguments du client#####
#si les arguments sont invalides, le programme doit
#afficher "usage: ..." et quitter avec une erreur
echo "test2: test des arguments du client"
TEST="test2"


./client                          2> $TMP >&2 || tu && fail "sans arg"
./client localhost                2> $TMP >&2 || tu && fail "sans port"
./client localhost 9000 x         2> $TMP >&2 || tu && fail "commande invalide"
./client localhost 9000 put       2> $TMP >&2 || tu && fail "put sans arg"
./client localhost 9000 put 1     2> $TMP >&2 || tu && fail "put sans adress"
./client localhost 9000 put 1 x   2> $TMP >&2 || tu && fail "put adresse invalide"
./client localhost 9000 put x ::1 2> $TMP >&2 || tu && fail "put hash invalide"
./client localhost 9000 get x     2> $TMP >&2 || tu && fail "get hash invalide"




#####test3: test d'utilisations simple des programmes#####
#les programmes soivent quitter sans erreur et afficher
#les bon résultats
echo "test3: de communication simple entre un client et un server"


#test3: communication simple entre client et server
echo "test3.1: put simple"
TEST="test3.1"
./server localhost 9000               >&2 &
sleep 0.1
./client localhost 9000 put a ::23    >&2                    || fail "client put"
sleep 0.1
./client localhost 9000 get a 2> $TMP >&2 && cmp_disp "::23" || fail "client get"
killall ./server


#test3.2: put et get dupliqués
echo "test3.2: put dupliquées"
TEST="test3.2"
./server localhost 9000               >&2 &
sleep 0.1
./client localhost 9000 put a ::12:12 >&2                           || fail "client premier put" 
./client localhost 9000 put a ::12:12 >&2                           || fail "client deuxieme put"
sleep 0.1
./client localhost 9000 get a 2> $TMP >&2 && cmp_disp "::0.18.0.18" || fail "client get"
killall ./server


#test3.3: put et get multiples
echo "test3.3: put multiples"
TEST="test3.3"
./server localhost 9000                 >&2 &
sleep 0.1
./client localhost 9000 put a ::ffff    >&2                                         || fail "client premier put"
./client localhost 9000 put a ::6f      >&2                                         || fail "client deuxieme put"
sleep 0.1
./client localhost 9000 get a   2> $TMP >&2 && cmp_disp "::6f" && cmp_disp "::ffff" || fail "client get"
killall ./server


#test3.4: connexion de servers
echo "test3.4: connexion de servers"
TEST="test3.4"
./server localhost 9000                 >&2 &
sleep 0.1
./server localhost 9001 localhost 9000  >&2 &
sleep 0.1
./server localhost 9002 localhost 9001  >&2 &
sleep 0.1
./client localhost 9000 put a ::ffff    >&2                      || fail "client put 9000"
sleep 0.1
./client localhost 9000 get a   2> $TMP >&2 && cmp_disp "::ffff" || fail "client get 9000"
./client localhost 9001 get a   2> $TMP >&2 && cmp_disp "::ffff" || fail "client get 9001"
./client localhost 9002 get a   2> $TMP >&2 && cmp_disp "::ffff" || fail "client get 9002"
killall ./server

#test3.5: obsolescnce des données
echo "test3.5: obsolescence des données (durée du test: un peu plus de 40 secondes)"
TEST="test3.5"
./server localhost 9000               >&2 &
sleep 0.1
./client localhost 9000 put a ::ffff  >&2                      || fail "client put"
sleep 0.1
./client localhost 9000 get a 2> $TMP >&2 && cmp_disp "::ffff" || fail "premier client get"
sleep 40
./client localhost 9000 get a 2> $TMP >&2 && cmp_disp "::ffff" && fail "deuxieme client get apres 40 secondes"
killall ./server




#####test4: test d'utilisations avancé des programmes#####
#les programmes soivent quitter sans erreur et afficher
#les bon résultats
echo "test4: avancé d'utilisation des programmes avec de nombreux servers (duréé: environ une minute)"


TEST="test4"
#connexion de servers
./server localhost 9000                  >&2 &
sleep 0.1
./server localhost 9001 localhost 9000   >&2 &
sleep 0.1
./server localhost 9002 localhost 9001   >&2 &
sleep 0.1
./server localhost 9003 localhost 9002   >&2 &
sleep 0.1
./server localhost 9004 localhost 9001   >&2 &
sleep 0.1
./server localhost 9005 localhost 9000   >&2 &
sleep 0.1
./server localhost 9006 localhost 9001   >&2 &
sleep 0.1
#envoi de donnée inexistante à un server
./client localhost 9001 get a    2> $TMP >&2 && cmp_disp "::" && fail "client get a, alors qu'aucune donnée n'est envoyé aux servers"
#envoi de données a un server
./client localhost 9000 put a ::aaff     >&2 || fail "client localhost 9000 put a ::aaff"
sleep 0.1
#verification que tous les servers ont bien reçu les données
./client localhost 9000 get a    2> $TMP >&2 && cmp_disp "::aaff" || fail "client localhost 9000 get a"
./client localhost 9001 get a    2> $TMP >&2 && cmp_disp "::aaff" || fail "client localhost 9001 get a"
./client localhost 9002 get a    2> $TMP >&2 && cmp_disp "::aaff" || fail "client localhost 9002 get a"
./client localhost 9003 get a    2> $TMP >&2 && cmp_disp "::aaff" || fail "client localhost 9003 get a"
./client localhost 9004 get a    2> $TMP >&2 && cmp_disp "::aaff" || fail "client localhost 9004 get a"
./client localhost 9005 get a    2> $TMP >&2 && cmp_disp "::aaff" || fail "client localhost 9005 get a"
./client localhost 9006 get a    2> $TMP >&2 && cmp_disp "::aaff" || fail "client localhost 9006 get a"
#envoi de nouvelles données
./client localhost 9002 put b ::bbee     >&2 || fail "client localhost 9002 put b ::eebb"
sleep 0.1
#connexion d'un servers supplementaire
./server localhost 9007 localhost 9004   >&2 &
sleep 0.1
#verification qu'il ai bien obtenu les données que les autres servers possèdaient déja
./client localhost 9007 get b    2> $TMP >&2 && cmp_disp "::bbee" || fail "client localhost 9007 get b"
./client localhost 9007 get a    2> $TMP >&2 && cmp_disp "::aaff" || fail "client localhost 9007 get a"
#envoi des données à ce sever
./client localhost 9007 put c ::cc77     >&2 || fail "client localhost 9007 put c ::cc77"
sleep 0.1
#verification que les autres servers ont bien reçu ce qu'on vient d'envoyer au derner server
./client localhost 9002 get c    2> $TMP >&2 && cmp_disp "::cc77" || fail "client localhost 9007 get c"
#attendre 20 secondes pour faire vieillir les hashs a, b et c
sleep 20
./client localhost 9003 put d ::d3e9     >&2 || fail "client localhost 9003 put d ::d3e9"
#attendre 15 secondes supplémentaires pour que les 3 premiers hashs soient obsolète
sleep 15
#verifier que a, b et c ont été supprimé et que d est toujours disponible
./client localhost 9002 get a    2> $TMP >&2 && cmp_disp "::aaff" && fail "client localhost 9002 get a obsolete"
./client localhost 9000 get b    2> $TMP >&2 && cmp_disp "::bbee" && fail "client localhost 9000 get b obsolete"
./client localhost 9007 get c    2> $TMP >&2 && cmp_disp "::cc77" && fail "client localhost 9007 get c obsolete"
./client localhost 9004 get d    2> $TMP >&2 && cmp_disp "::d3e9" || fail "client localhost 9004 get d pas obsolete"
#attendre  20 secondes supplémentaires que d soit obsolète
sleep 20
./client localhost 9005 get d    2> $TMP >&2 && cmp_disp "::d3e9" && fail "client localhost 9005 get d obsolète"
killall ./server


echo "tous les tests sont passés avec succès !"




#####fin des tests#####
#suppression des fichier temporaires et terminaison des servers

rm -f $TMP

exit 0
