use core::{
    pin::Pin,
    task::{Context, Poll},
    future::Future,
    convert::AsRef,
};
use alloc::{
    boxed::Box,
    vec::Vec,
};

pub use futures_util::io::{AsyncRead, AsyncWrite};

use crate::{
    io,
    ep_server::EP_SERVER,
    rpc::RpcClient,
    ns::ns_client,
    path::{Path, PathBuf},
};
use rustyl4api::object::EpCap;

pub struct File {
    client: RpcClient,
    offset: usize,
}

impl File {
    pub async fn open<P: AsRef<Path>>(path: P) -> Result<Self, ()> {

        let resp = ns_client().lock().lookup_service(path.as_ref()).await.map_err(|_|())?;
        Self::connect(EpCap::new(resp)).await
    }

    pub async fn connect(ep: EpCap) -> Result<Self, ()> {
        let ep_server = EP_SERVER.try_get().unwrap();
        let (ntf_badge, ntf_ep) = ep_server.derive_badged_cap().unwrap();
        let cli = RpcClient::connect(
            ep,
            ntf_ep,
            ntf_badge
        ).unwrap();
        Ok(Self {
            client: cli,
            offset: 0,
        })
    }

    pub async fn read_dir(&mut self) -> Result<Vec<PathBuf>, ()> {
        self.client.read_dir().await.map_err(|_| ())
    }
}

impl AsyncWrite for File {
    fn poll_write(
        mut self: Pin<&mut Self>,
        cx: &mut Context<'_>,
        buf: &[u8],
    ) -> Poll<io::Result<usize>> {
        let client = &mut self.client;
        let mut fut = Box::pin(client.rpc_write(buf));
        Pin::new(&mut fut).poll(cx).map(|r| Ok(r))
    }

    fn poll_flush(self: Pin<&mut Self>, _cx: &mut Context<'_>) -> Poll<io::Result<()>> {
        Poll::Ready(Ok(()))
    }

    fn poll_close(self: Pin<&mut Self>, _cx: &mut Context<'_>) -> Poll<io::Result<()>> {
        Poll::Ready(Ok(()))
    }
}

impl AsyncRead for File {
    fn poll_read(
        mut self: Pin<&mut Self>,
        cx: &mut Context<'_>,
        buf: &mut [u8],
    ) -> Poll<io::Result<usize>> {
        let Self { client, offset } = &mut *self;
        let mut fut = Box::pin(client.rpc_read(buf, *offset));
        Pin::new(&mut fut).poll(cx).map(|readlen| {
            *offset += readlen;
            Ok(readlen)
        })
    }
}