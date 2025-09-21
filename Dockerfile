FROM greyltc/archlinux-aur:paru AS build-dependencies
RUN echo 'MAKEFLAGS="-j$(nproc)"' >> /etc/makepkg.conf
USER ab
WORKDIR /var/ab/pkg/arch
COPY --chown=ab:ab ./pkg/arch/ /var/ab/pkg/arch/
RUN --mount=type=cache,target=/var/ab/.cache/paru,sharing=private,uid=971,gid=971 \
	--mount=type=cache,target=/var/cache/pacman/pkg,sharing=private \
	./build-deps.sh

FROM archlinux:base-devel AS install-dependecies
WORKDIR /fdtd-lucuma
RUN \
	--mount=type=bind,from=build-dependencies,source=/var/ab/pkg/arch,target=/fdtd-lucuma \
	--mount=type=bind,from=build-dependencies,source=/var/cache/makepkg/pkg/,target=/var/cache/makepkg/pkg/ \
	--mount=type=cache,target=/var/cache/pacman/pkg,sharing=private \
	./install-deps.sh vulkan-validation-layers vulkan-radeon #TODO: Change driver by target

FROM install-dependecies AS build
WORKDIR /fdtd-lucuma
RUN --mount=type=bind,source=.,target=./source \
	--mount=type=cache,target=./build,sharing=private \
	cmake \
		-DUSE_SYSTEM_LIBS=ON \
		-B build \
		-G Ninja \
		-S source && \
	cmake --build build --parallel $(nproc) && \
	cp -r build build_copy

FROM install-dependecies AS exec
WORKDIR /fdtd-lucuma/build
COPY --from=build /fdtd-lucuma/build_copy ./
CMD ["./fdtd-lucuma"]
