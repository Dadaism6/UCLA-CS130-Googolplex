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

# Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/webserver .
COPY --from=builder /usr/src/project/config/default.config .
COPY --from=builder /usr/src/project/static .

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
# Update with real server name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
# Update with real server args
CMD ["default.config"]
