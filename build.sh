rustup override set nightly
rustup component add rust-src --toolchain nightly-x86_64-unknown-linux-gnu
rustup component add clippy --toolchain nightly-x86_64-unknown-linux-gnu
cargo build