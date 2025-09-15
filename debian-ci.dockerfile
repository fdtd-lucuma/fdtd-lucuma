FROM debian:experimental
WORKDIR /fdtd-vulkan
RUN apt update && \
	apt install -y git
COPY ./pkg/ubuntu/ /fdtd-vulkan/pkg/ubuntu/
RUN xargs -a pkg/ubuntu/dependencies.txt -- apt -t experimental install -y
RUN \
	apt install -y curl zip unzip tar && \
	git clone https://github.com/microsoft/vcpkg.git && \
	cd vcpkg && ./bootstrap-vcpkg.sh -disableMetrics && \
	./vcpkg install shader-slang glm
