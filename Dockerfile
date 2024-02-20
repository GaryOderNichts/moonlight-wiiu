FROM devkitpro/devkitppc:20240220

# build latest wut from source
RUN \
mkdir wut && \
cd wut && \
git init . && \
git remote add origin https://github.com/devkitPro/wut.git && \
git fetch --depth 1 origin c1115e51bb16979a04463e2bf2ebc4369a013e67 && \
git checkout FETCH_HEAD
WORKDIR /wut
RUN make -j$(nproc)
RUN make install

WORKDIR /project
