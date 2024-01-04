rustup target add aarch64-unknown-linux-gnu
rustup override set nightly
rustup component add rust-src --toolchain nightly-x86_64-unknown-linux-gnu
sudo apt-get install aarch64-none-elf binutils
cd kernel
make
