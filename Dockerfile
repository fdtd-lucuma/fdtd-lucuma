FROM greyltc/archlinux-aur:paru AS dependencies
RUN echo 'MAKEFLAGS="-j$(nproc)"' >> /etc/makepkg.conf
WORKDIR /var/ab/pkg/arch
USER ab
COPY ./pkg/arch /var/ab/pkg/arch
RUN --mount=type=cache,target=/var/ab/.cache/paru,sharing=private,uid=971,gid=971 \
	./build-deps.sh

FROM archlinux:base-devel AS build
WORKDIR /fdtd-vulkan/pkg/arch
COPY ./pkg/arch .
RUN --mount=type=bind,from=dependencies,source=/var/cache/makepkg/pkg/,target=/var/cache/makepkg/pkg/ \
	./install-deps.sh
WORKDIR /fdtd-vulkan
COPY . .
RUN cmake -B build -G Ninja
RUN cmake --build build --parallel $(nproc)

FROM build AS exec
WORKDIR /fdtd-vulkan/build
