FROM gcc:9.2.0 AS Builder
COPY . /usr/src/myapp/
WORKDIR /usr/src/myapp/
RUN gcc -static -o myapp main.c

FROM alpine:latest
COPY --from=Builder /usr/src/myapp/myapp .

CMD ["/myapp"]

