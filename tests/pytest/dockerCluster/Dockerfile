FROM ubuntu:latest AS builder
 
ARG PACKAGE=TDengine-server-1.6.5.10-Linux-x64.tar.gz
ARG EXTRACTDIR=TDengine-enterprise-server
ARG TARBITRATORPKG=TDengine-tarbitrator-1.6.5.10-Linux-x64.tar.gz
ARG EXTRACTDIR2=TDengine-enterprise-arbitrator
ARG CONTENT=taos.tar.gz
 
WORKDIR /root

COPY ${PACKAGE} .
COPY ${TARBITRATORPKG} .
 
RUN tar -zxf ${PACKAGE}
RUN tar -zxf ${TARBITRATORPKG}
RUN mv ${EXTRACTDIR}/driver ./lib
RUN tar -zxf ${EXTRACTDIR}/${CONTENT}
RUN mv ${EXTRACTDIR2}/bin/* /root/bin
 
FROM ubuntu:latest
 
WORKDIR /root

RUN apt-get update
RUN apt-get install -y vim tmux net-tools
RUN echo 'alias ll="ls -l --color=auto"' >> /root/.bashrc
RUN ulimit -c unlimited
       
COPY --from=builder /root/bin/taosd /usr/bin
COPY --from=builder /root/bin/tarbitrator /usr/bin
COPY --from=builder /root/bin/taosdemo /usr/bin
COPY --from=builder /root/bin/taosdump /usr/bin
COPY --from=builder /root/bin/taos /usr/bin
COPY --from=builder /root/cfg/taos.cfg /etc/taos/
COPY --from=builder /root/lib/libtaos.so.* /usr/lib/libtaos.so.1
COPY --from=builder /root/lib/libtaosnative.so.* /usr/lib/libtaosnative.so.1
 
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/lib"
ENV LC_CTYPE=en_US.UTF-8
ENV LANG=en_US.UTF-8
 
EXPOSE 6030-6042/tcp 6060/tcp 6030-6039/udp
 
# VOLUME [ "/var/lib/taos", "/var/log/taos", "/etc/taos" ]
 
CMD [ "bash" ]