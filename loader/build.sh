rm debug

if [ -n "$RANDOM_NUMBER_OVERRIDE" ]; then
    random_number="$RANDOM_NUMBER_OVERRIDE"
else
    random_number=$(hexdump -n 16 -e '16/1 "%02X" "\n"' /dev/urandom)
fi

loader_user_email="$LOADER_USER_EMAIL"
loader_user_access_token="$LOADER_USER_ACCESS_TOKEN"
build_timestamp=$(date +%Y-%m-%d_%H-%M-%S)

echo "SEED: $random_number"
echo "LOADER USER EMAIL: $loader_user_email"
echo "LOADER USER ACCESS TOKEN: $loader_user_access_token"
echo "BUILDING ON $build_timestamp"

/src/obfuscator-drause-llvm11/build/bin/clang++ \
-m64 -std=c++17 src/*.cpp -lm -O0 -no-pie -Wno-multichar -s -fno-rtti -Wl,-allow-multiple-definition \
-lssl -lcrypto -lelf -l:libsodium.a -llzma \
-msse4.2 -o debug \
-DR5I_PROD \
-mllvm -aesSeed="$random_number" -mllvm -csobf -mllvm -epobf -mllvm -sub -mllvm -sub_loop=3 \
-D_LOADER_COMPILATION_AES_KEY="$random_number" \
-D_LOADER_USER_EMAIL="$loader_user_email" \
-D_LOADER_USER_ACCESS_TOKEN="$loader_user_access_token" \
-D_BUILD_TIMESTAMP=$build_timestamp \
-I/home/drof/CLionProjects/ -Iprojects/ -I/src/obfuscator-drause-llvm11/build/lib/clang/11.0.0/include/
/src/obfuscator-drause-llvm11/build/bin/llvm-strip debug