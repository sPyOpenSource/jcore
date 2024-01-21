rustup target add aarch64-unknown-linux-gnu
rustup override set nightly
rustup component add rust-src
sudo apt-get install gcc-aarch64-linux-gnu binutils
cd kernel
cargo install xargo
xargo build --target=aarch64-none-elf
