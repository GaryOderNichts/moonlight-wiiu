# build wut
FROM devkitpro/devkitppc:20220128 AS wutbuild

ENV PATH=$DEVKITPPC/bin:$PATH

WORKDIR /
RUN git clone https://github.com/devkitPro/wut
WORKDIR /wut
RUN make -j$(nproc)
RUN make install
WORKDIR /

# set up builder image
FROM devkitpro/devkitppc:20220128 AS builder

RUN apt-get update && apt-get -y install --no-install-recommends wget tar autoconf automake libtool && rm -rf /var/lib/apt/lists/*
COPY --from=wutbuild /opt/devkitpro/wut /opt/devkitpro/wut

# build SDL2
FROM builder AS sdlbuild
ENV WUT_ROOT=$DEVKITPRO/wut

RUN git clone -b wiiu-2.0.9 --single-branch https://github.com/yawut/SDL
WORKDIR /SDL
RUN mkdir build
WORKDIR /SDL/build
RUN /opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake .. -DCMAKE_INSTALL_PREFIX=$DEVKITPRO/portlibs/wiiu
RUN make -j$(nproc) && make install
WORKDIR /

# build openssl
FROM builder AS opensslbuild
ARG openssl_ver=1.1.1m

RUN wget https://www.openssl.org/source/openssl-$openssl_ver.tar.gz && mkdir /openssl && tar xf openssl-$openssl_ver.tar.gz -C /openssl --strip-components=1
WORKDIR /openssl

RUN echo 'diff --git a/Configurations/10-main.conf b/Configurations/10-main.conf\n\
index 61c6689..efe686a 100644\n\
--- a/Configurations/10-main.conf\n\
+++ b/Configurations/10-main.conf\n\
@@ -627,6 +627,27 @@ my %targets = (\n\
         shared_extension => ".so",\n\
     },\n\
 \n\
+### Wii U target\n\
+    "wiiu" => {\n\
+        inherit_from     => [ "BASE_unix" ],\n\
+        CC               => "$ENV{DEVKITPPC}/bin/powerpc-eabi-gcc",\n\
+        CXX              => "$ENV{DEVKITPPC}/bin/powerpc-eabi-g++",\n\
+        AR               => "$ENV{DEVKITPPC}/bin/powerpc-eabi-ar",\n\
+        CFLAGS           => picker(default => "-Wall",\n\
+                                   debug   => "-O0 -g",\n\
+                                   release => "-O3"),\n\
+        CXXFLAGS         => picker(default => "-Wall",\n\
+                                   debug   => "-O0 -g",\n\
+                                   release => "-O3"),\n\
+        LDFLAGS          => "-L$ENV{DEVKITPRO}/wut/lib",\n\
+        cflags           => add("-mcpu=750 -meabi -mhard-float -ffunction-sections -fdata-sections"),\n\
+        cxxflags         => add("-std=c++11"),\n\
+        lib_cppflags     => "-DOPENSSL_USE_NODELETE -DB_ENDIAN -DNO_SYS_UN_H -DNO_SYSLOG -D__WIIU__ -D__WUT__ -I$ENV{DEVKITPRO}/wut/include",\n\
+        ex_libs          => add("-lwut -lm"),\n\
+        bn_ops           => "BN_LLONG RC4_CHAR",\n\
+        asm_arch         => '"'"'ppc32'"'"',\n\
+    },\n\
+\n ####\n #### Variety of LINUX:-)\n ####\n\
diff --git a/crypto/rand/rand_unix.c b/crypto/rand/rand_unix.c\n\
index 0f45251..d303e8e 100644\n\
--- a/crypto/rand/rand_unix.c\n\
+++ b/crypto/rand/rand_unix.c\n\
@@ -202,6 +202,41 @@ void rand_pool_keep_random_devices_open(int keep)\n\
 {\n\
 }\n\
 \n\
+# elif defined(__WIIU__)\n\
+\n\
+#include <coreinit/time.h>\n\
+\n\
+size_t rand_pool_acquire_entropy(RAND_POOL *pool)\n\
+{\n\
+    int i;\n\
+    size_t bytes_needed;\n\
+    unsigned char v;\n\
+\n\
+    bytes_needed = rand_pool_bytes_needed(pool, 4 /*entropy_factor*/);\n\
+\n\
+    for (i = 0; i < bytes_needed; i++) {\n\
+        srand(OSGetSystemTick());\n\
+        v = rand() & 0xff;\n\
+\n\
+        rand_pool_add(pool, &v, sizeof(v), 2);\n\
+    }\n\
+\n\
+    return rand_pool_entropy_available(pool);\n\
+}\n\
+\n\
+int rand_pool_init(void)\n\
+{\n\
+    return 1;\n\
+}\n\
+\n\
+void rand_pool_cleanup(void)\n\
+{\n\
+}\n\
+\n\
+void rand_pool_keep_random_devices_open(int keep)\n\
+{\n\
+}\n\
+\n # else\n\
 \n #  if defined(OPENSSL_RAND_SEED_EGD) && \\\n\
diff --git a/crypto/uid.c b/crypto/uid.c\n\
index a9eae36..4a81d98 100644\n\
--- a/crypto/uid.c\n\
+++ b/crypto/uid.c\n\
@@ -10,7 +10,7 @@\n #include <openssl/crypto.h>\n #include <openssl/opensslconf.h>\n\
 \n\
-#if defined(OPENSSL_SYS_WIN32) || defined(OPENSSL_SYS_VXWORKS) || defined(OPENSSL_SYS_UEFI)\n\
+#if defined(OPENSSL_SYS_WIN32) || defined(OPENSSL_SYS_VXWORKS) || defined(OPENSSL_SYS_UEFI) || defined(__WIIU__)\n\
 \n\
 int OPENSSL_issetugid(void)\n\
 {\
' >> wiiu.patch && git apply wiiu.patch

RUN ./Configure wiiu \
  no-threads no-shared no-asm no-ui-console no-unit-test no-tests no-buildtest-c++ no-external-tests no-autoload-config \
  --with-rand-seed=os -static

RUN make build_generated
RUN make libssl.a libcrypto.a -j$(nproc)
WORKDIR /

# build curl
FROM builder as curlbuild
ARG curl_ver=7.81.0

# copy in openssl
COPY --from=opensslbuild /openssl/libcrypto.a /openssl/libssl.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=opensslbuild /openssl/include/openssl /opt/devkitpro/portlibs/wiiu/include/openssl/
COPY --from=opensslbuild /openssl/include/crypto /opt/devkitpro/portlibs/wiiu/include/crypto/

RUN wget https://curl.se/download/curl-$curl_ver.tar.gz && mkdir /curl && tar xf curl-$curl_ver.tar.gz -C /curl --strip-components=1
WORKDIR /curl

ENV CFLAGS "-mcpu=750 -meabi -mhard-float -O3 -ffunction-sections -fdata-sections"
ENV CXXFLAGS "${CFLAGS}"
ENV CPPFLAGS "-D__WIIU__ -D__WUT__ -I${DEVKITPRO}/wut/include"
ENV LDFLAGS "-L${DEVKITPRO}/wut/lib"
ENV LIBS "-lwut -lm"

RUN autoreconf -fi
RUN ./configure \
--prefix=$DEVKITPRO/portlibs/wiiu/ \
--host=powerpc-eabi \
--enable-static \
--disable-threaded-resolver \
--disable-pthreads \
--with-ssl=$DEVKITPRO/portlibs/wiiu/ \
--disable-ipv6 \
--disable-unix-sockets \
--disable-socketpair \
--disable-ntlm-wb \
CC=$DEVKITPPC/bin/powerpc-eabi-gcc \
AR=$DEVKITPPC/bin/powerpc-eabi-ar \
RANLIB=$DEVKITPPC/bin/powerpc-eabi-ranlib \
PKG_CONFIG=$DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-pkg-config

WORKDIR /curl/lib
RUN make -j$(nproc) install
WORKDIR /curl/include
RUN make -j$(nproc) install
WORKDIR /

FROM builder as expatbuild
ARG expat_tag=2_4_0
ARG expat_ver=2.4.0

RUN wget https://github.com/libexpat/libexpat/releases/download/R_$expat_tag/expat-$expat_ver.tar.gz && mkdir /expat && tar xf expat-$expat_ver.tar.gz -C /expat --strip-components=1
WORKDIR /expat

ENV CFLAGS "-mcpu=750 -meabi -mhard-float -O3 -ffunction-sections -fdata-sections"
ENV CXXFLAGS "${CFLAGS}"
ENV CPPFLAGS "-D__WIIU__ -D__WUT__ -I${DEVKITPRO}/wut/include"
ENV LDFLAGS "-L${DEVKITPRO}/wut/lib"
ENV LIBS "-lwut -lm"

RUN autoreconf -fi
RUN ./configure \
--prefix=$DEVKITPRO/portlibs/wiiu/ \
--host=powerpc-eabi \
--enable-static \
--without-examples \
--without-tests \
--without-docbook \
CC=$DEVKITPPC/bin/powerpc-eabi-gcc \
AR=$DEVKITPPC/bin/powerpc-eabi-ar \
RANLIB=$DEVKITPPC/bin/powerpc-eabi-ranlib \
PKG_CONFIG=$DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-pkg-config

RUN make -j$(nproc) && make install
WORKDIR /

FROM builder as opusbuild
ARG opus_ver=1.1.2

RUN wget https://github.com/xiph/opus/releases/download/v$opus_ver/opus-$opus_ver.tar.gz && mkdir /opus && tar xf opus-$opus_ver.tar.gz -C /opus --strip-components=1
WORKDIR /opus

ENV CFLAGS "-mcpu=750 -meabi -mhard-float -O3 -ffunction-sections -fdata-sections"
ENV CXXFLAGS "${CFLAGS}"
ENV CPPFLAGS "-D__WIIU__ -D__WUT__ -I${DEVKITPRO}/wut/include"
ENV LDFLAGS "-L${DEVKITPRO}/wut/lib"
ENV LIBS "-lwut -lm"

RUN ./configure \
--prefix=$DEVKITPRO/portlibs/wiiu/ \
--host=powerpc-eabi \
--enable-static \
--disable-doc \
--disable-extra-programs \
CC=$DEVKITPPC/bin/powerpc-eabi-gcc \
AR=$DEVKITPPC/bin/powerpc-eabi-ar \
RANLIB=$DEVKITPPC/bin/powerpc-eabi-ranlib \
PKG_CONFIG=$DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-pkg-config \
CFLAGS="$CFLAGS -Wno-expansion-to-defined"
RUN make -j$(nproc) && make install

# build final container
FROM devkitpro/devkitppc:20220128 AS final

# copy in wut
COPY --from=wutbuild /opt/devkitpro/wut /opt/devkitpro/wut

# copy in SDL2
COPY --from=sdlbuild /opt/devkitpro/portlibs/wiiu/lib/libSDL2.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=sdlbuild /opt/devkitpro/portlibs/wiiu/include/SDL2 /opt/devkitpro/portlibs/wiiu/include/SDL2/

# copy in openssl
COPY --from=opensslbuild /openssl/libcrypto.a /openssl/libssl.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=opensslbuild /openssl/include/openssl /opt/devkitpro/portlibs/wiiu/include/openssl/
COPY --from=opensslbuild /openssl/include/crypto /opt/devkitpro/portlibs/wiiu/include/crypto/

# copy in curl
COPY --from=curlbuild /opt/devkitpro/portlibs/wiiu/lib/libcurl.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=curlbuild /opt/devkitpro/portlibs/wiiu/include/curl /opt/devkitpro/portlibs/wiiu/include/curl/

# copy in expat
COPY --from=expatbuild /opt/devkitpro/portlibs/wiiu/lib/libexpat.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=expatbuild /opt/devkitpro/portlibs/wiiu/include/expat.h /opt/devkitpro/portlibs/wiiu/include/expat.h
COPY --from=expatbuild /opt/devkitpro/portlibs/wiiu/include/expat_config.h /opt/devkitpro/portlibs/wiiu/include/expat_config.h
COPY --from=expatbuild /opt/devkitpro/portlibs/wiiu/include/expat_external.h /opt/devkitpro/portlibs/wiiu/include/expat_external.h

# copy in opus
COPY --from=opusbuild /opt/devkitpro/portlibs/wiiu/lib/libopus.a /opt/devkitpro/portlibs/wiiu/lib/
COPY --from=opusbuild /opt/devkitpro/portlibs/wiiu/include/opus /opt/devkitpro/portlibs/wiiu/include/opus/

WORKDIR /project
