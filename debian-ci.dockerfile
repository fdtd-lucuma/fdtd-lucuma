FROM debian:experimental
WORKDIR /fdtd-vulkan
RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
	--mount=type=cache,target=/var/cache/apt,sharing=locked \
	apt update && \
	apt install -y git && \
	apt install -y curl zip unzip tar cmake ninja-build build-essential && \
	git clone https://github.com/microsoft/vcpkg.git /vcpkg && \
	cd /vcpkg && ./bootstrap-vcpkg.sh -disableMetrics
RUN /vcpkg/vcpkg install shader-slang glm
COPY ./pkg/ubuntu/ /fdtd-vulkan/pkg/ubuntu/
RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
	--mount=type=cache,target=/var/cache/apt,sharing=locked \
	apt install -y ccache && \
	xargs -a pkg/ubuntu/dependencies.txt -- apt -t experimental install -y
