set -xe
cd executables/games_executables

for i in {1..5}; do
    echo "--- Esecuzione $i ---"
    ./med_test

    echo "Pausa di raffreddamento..."
    sleep 15
done

cd ../..
