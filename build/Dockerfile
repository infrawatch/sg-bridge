# --- Build the bridge
FROM registry.access.redhat.com/ubi8 AS builder

# dependencies for qpid-proton-c
COPY build/repos/opstools.repo /etc/yum.repos.d/opstools.repo

# redhat-rpm-config is required to provide hardening compiling instructions
# (such as /usr/lib/rpm/redhat/redhat-hardened-cc1) even though we're not
# building RPMs here
RUN dnf install qpid-proton-c-devel --setopt=tsflags=nodocs -y && \
        dnf install gcc make redhat-rpm-config -y && \
        dnf clean all

ENV D=/home/bridge

WORKDIR $D
COPY . $D/

RUN     make && \
        mv bridge /tmp/

# --- end build, create bridge runtime layer ---
FROM registry.access.redhat.com/ubi8

# dependencies for qpid-proton-c
COPY build/repos/opstools.repo /etc/yum.repos.d/opstools.repo

RUN dnf install qpid-proton-c --setopt=tsflags=nodocs -y && \
        dnf clean all && \
        rm -rf /var/cache/yum

COPY --from=builder /tmp/bridge /

ENTRYPOINT ["/bridge"]

LABEL io.k8s.display-name="SmartGateway AMQP Bridge" \
      io.k8s.description="Reads data from AMQP via proton and writes to a unix or UDP socket"
