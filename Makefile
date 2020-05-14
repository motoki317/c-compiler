buildImage:
	docker build -t compilerbook .

docker:
	docker run --rm -it -w /home/user/compiler -v $(PWD)/compiler:/home/user/compiler compilerbook bash
