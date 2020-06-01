.PHONY: buildImage
buildImage:
	docker build -t compilerbook .

.PHONY: docker
docker:
	docker run --rm -it -w /home/user/compiler -v $(PWD)/compiler:/home/user/compiler compilerbook bash
