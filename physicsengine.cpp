#include "physicsengine.h"

PhysicsEngine::PhysicsEngine() : CLEngine() {}

void PhysicsEngine::Init() {
	_init();

	int err;
	InitializeProgram("physics.cl");
	
	queue = clCreateCommandQueue(context, device, 0, &err);
	if(err < 0) { perror("Couldn't create a command queue"); exit(1); }

	kernels.push_back(cl_kernel());
	kernels[0] = clCreateKernel(program, "updateVelocity", &err);
	if(err < 0) { perror("Couldn't create a kernel"); exit(1); }	

	kernels.push_back(cl_kernel());
	kernels[1] = clCreateKernel(program, "applyVelocity", &err);
	if(err < 0) { perror("Couldn't create a kernel"); exit(1); }

	input0 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(vec4<float>), NULL, &err);
	if(err < 0) { perror("Could not create vec4 buffer."); exit(1); }
	input4 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int), NULL, &err);
	if(err < 0) { perror("Could not create int buffer."); exit(1); }
	

}

void PhysicsEngine::Step(Object & obj) {
	int err;
	
	glFinish();

	cl_event events[2];

	const size_t osize = obj.position.size();
	const size_t globalNum = osize;
	size_t localNum = work_group_size;
	if( osize < work_group_size )
		localNum = osize;
	const size_t numGroups = (globalNum/localNum)+1;
	cl_mem input5 = clCreateBuffer(context, CL_MEM_READ_WRITE, numGroups*sizeof(vec4<float>), NULL, &err);
	if(err < 0) { perror("Couldn't create comm buffer(input5).\n"); }

	if(obj.cl_vbo_mem == 0 ) { return; }
	err = clEnqueueAcquireGLObjects(queue, 1, &obj.cl_vbo_mem, 0, NULL, NULL );
	if(err < 0) { perror("Acquire GLObject failed."); }

	//Returns if pointer is null
	if(obj.cl_vbo_mem == 0) { return; }
	if(obj.cl_vel_mem == 0) { return; }
	cl_kernel kernel = kernels[0];
	for( int i=0; i<osize; i++) {
//		fprintf(stderr, "pos addr: %i\n", &obj.position[i]);
//		for( int j=0; j<4; j++)
//			fprintf(stderr, "%f ", obj.position[i].data[j]);
//		fprintf(stderr, "\n");
//Added input buffers because the commented kernel args would segfault on second loop.
//		clEnqueueWriteBuffer(queue, input0, CL_FALSE, 0, sizeof(vec4<float>), &obj.position[i], 0, NULL, NULL);
		clEnqueueWriteBuffer(queue, input4, CL_FALSE, 0, sizeof(int), &i, 0, NULL, NULL);
//		err  = clSetKernelArg(kernel, 0, sizeof(float), &obj.position[i]);
		err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input0);
		err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &obj.cl_vbo_mem);
		err |= clSetKernelArg(kernel, 2, 4*sizeof(float)*localNum, NULL);
		err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &obj.cl_vel_mem);
//		err |= clSetKernelArg(kernel, 4, sizeof(int), &i);
		err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &input4);
		err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &input5);

//		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &obj.cl_vbo_mem);
//		err |= clSetKernelArg(kernel, 1, 4*sizeof(float)*osize, NULL);
//		err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &obj.cl_vbo_mem);
		
		if(err != CL_SUCCESS) { perror("Error setting kernel0 arguments."); }
		
		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalNum, &localNum, 0, NULL, NULL);
//		fprintf(stderr,"%i\n",err);
		if(err != CL_SUCCESS) { perror("Error queuing kernel0 for execution."); exit(1); }
	}

	kernel = kernels[1];
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &obj.cl_vbo_mem);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &obj.cl_vel_mem);
	if(err != CL_SUCCESS) { perror("Error setting kernel1 arguments."); }

	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalNum, &localNum, 0, NULL, NULL);
	if(err != CL_SUCCESS) { perror("Error queuing kernel1 for execution."); exit(1); }


	err = clEnqueueReleaseGLObjects(queue, 1, &obj.cl_vbo_mem, 0, NULL, NULL);
	if(err < 0) { perror("Couldn't release GLObject."); }

	err = clReleaseMemObject(input5);
	if(err < 0) { perror("Couldn't release memory object.(input5)"); }

	clFinish(queue);

/*
	glBindBuffer(GL_ARRAY_BUFFER, obj.getPBO());
	glGetBufferSubData(GL_ARRAY_BUFFER, 0,sizeof(vec4<float>)*osize,obj.position.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
*/
}


