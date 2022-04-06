### Build/test container ###
# Define builder stage
FROM googolplex:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:focal as deploy

# TODO(!): Copy server output binary to "."
# COPY --from=builder /usr/src/project/build/bin/example_server .

# TODO(!): Expose some port(s)
# EXPOSE 80

# Use ENTRYPOINT to specify the binary name
# TODO(!): Update with real server name
# ENTRYPOINT ["./example_server"]

# Use CMD to specify arguments to ENTRYPOINT
# TODO(!): Update with real server args
# CMD [""]
