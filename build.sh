rustup target add riscv64gc-unknown-none-elf
rustup override set nightly
rustup component add rust-src
cd risc_v
cargo build
