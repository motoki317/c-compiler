buildImage:
	docker build -t compilerbook .

docker:
	docker run --rm -it -v $(PWD):/home/user/compiler compilerbook
