echo "[SETUP] ------( CLEANUP )------"
rm -rf /src/hiredis/
rm -rf /src/redis-plus-plus/
rm -rf /src/libsodium-stable/

echo "[SETUP] ------( PACKAGE MANAGER EZ DEPENDENCIES )------"
apt install liblzma-dev || exit 1
apt install libelf-dev || exit 1
apt install libssl-dev | exit 1

echo "[SETUP] (SUCCESS) PACKAGE MANAGER EZ DEPENDENCIES"

##############################################################################

echo "[SETUP] ------( HIREDIS )------"
cd /src || exit 1

echo "[SETUP] cloning..."
git clone --recursive https://github.com/redis/hiredis
if [ $? -ne 0 ]; then
    echo "[SETUP] (ERROR) cloning failed. aborting!"
    exit 1
fi

cd hiredis ||exit 1

echo "[SETUP] making..."
make -j 6
if [ $? -ne 0 ]; then
    echo "[SETUP] (error) make failed. Aborting."
    exit 1
fi


echo "[SETUP] installing..."
make install
if [ $? -ne 0 ]; then
    echo "[SETUP] (error) installation failed. Aborting."
    exit 1
fi

echo "[SETUP] (SUCCESS) HIREDIS";
cd .. || exit 1

#############################################################################

echo "[SETUP] ------( REDIS++ )------";

git clone https://github.com/sewenew/redis-plus-plus.git
if [ $? -ne 0 ]; then
    echo "[SETUP] (ERROR) cloning failed. aborting!"
    exit 1
fi

cd redis-plus-plus || exit 1

mkdir build || exit 1
cd build || exit 1
cmake ..
if [ $? -ne 0 ]; then
    echo "[SETUP] (ERROR) cmake call has failed. aborting!"
    exit 1
fi

make -j 6
if [ $? -ne 0 ]; then
    echo "[SETUP] (error) make failed. Aborting."
    exit 1
fi

make install
if [ $? -ne 0 ]; then
    echo "[SETUP] (error) installation failed. Aborting."
    exit 1
fi

cd .. || exit 1

echo "[SETUP] (SUCCESS) REDIS++";
cd .. || exit 1

#############################################################################

echo "[SETUP] ------( LIBSODIUM )------"

wget https://download.libsodium.org/libsodium/releases/libsodium-1.0.18-stable.tar.gz || exit 1
tar -xzf libsodium-1.0.18-stable.tar.gz || exit 1
rm libsodium-1.0.18-stable.tar.gz || exit 1
cd libsodium-stable || exit 1
./configure
make -j 6

make check

make install

echo "[SETUP] (SUCCESS) LIBSODIUM"
cd ..

#############################################################################

echo "[SETUP] done."
