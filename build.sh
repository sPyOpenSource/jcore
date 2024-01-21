rustup override set nightly
rustup target add riscv64gc-unknown-none-elf
rustup component add rust-src
cd risc_v
cargo build
