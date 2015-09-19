/*!
 *  Copyright (c) 2015 by Contributors
 * \file c_api.h
 * \brief C API of mxnet
 */
#ifndef MXNET_C_API_H_
#define MXNET_C_API_H_

#ifdef __cplusplus
#define MXNET_EXTERN_C extern "C"
#endif

/*! \brief MXNET_DLL prefix for windows" */
#ifdef _MSC_VER
#define MXNET_DLL MXNET_EXTERN_C __declspec(dllexport)
#else
#define MXNET_DLL MXNET_EXTERN_C
#endif

/*! \brief manually define unsigned int */
typedef unsigned int mx_uint;
/*! \brief manually define unsigned long int */
typedef unsigned long int mx_ulong;  // NOLINT(*)
/*! \brief manually define unsigned int */
typedef float mx_float;
// all the handles are simply void *
// will be casted internally to specific pointers types
// these typedefs are mainly used for readablity reasons
/*! \brief handle to NDArray */
typedef void *NDArrayHandle;
/*! \brief handle to a mxnet narray function that changes NDArray */
typedef const void *FunctionHandle;
/*! \brief handle to a function that takes param and creates symbol */
typedef void *AtomicSymbolCreator;
/*! \brief handle to a symbol that can be bind as operator */
typedef void *SymbolHandle;
/*! \brief handle to a AtomicSymbol */
typedef void *AtomicSymbolHandle;
/*! \brief handle to an Executor */
typedef void *ExecutorHandle;
/*! \brief handle a dataiter creator */
typedef void *DataIterCreator;
/*! \brief handle to a DataIterator */
typedef void *DataIterHandle;
/*!
 * \brief return str message of the last error
 *  all function in this file will return 0 when success
 *  and -1 when an error occured,
 *  MXGetLastError can be called to retrieve the error
 *
 *  this function is threadsafe and can be called by different thread
 *  \return error info
 */
MXNET_DLL const char *MXGetLastError();

//-------------------------------------
// Part 0: Global State setups
//-------------------------------------
/*!
 * \brief Seed the global random number generators in mxnet.
 * \param seed the random number seed.
 * \return 0 when success, -1 when failure happens.
 */
MXNET_DLL int MXRandomSeed(int seed);
//-------------------------------------
// Part 1: NDArray creation and deletion
//-------------------------------------
/*!
 * \brief create a NDArray handle that is not initialized
 *  can be used to pass in as mutate variables
 *  to hold the result of NDArray
 * \param out the returning handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayCreateNone(NDArrayHandle *out);
/*!
 * \brief create a NDArray with specified shape
 * \param shape the pointer to the shape
 * \param ndim the dimension of the shape
 * \param dev_mask device mask, specify device we want to take
 * \param dev_id the device id of the specific device
 * \param delay_alloc whether to delay allocation until
 *    the narray is first mutated
 * \param out the returning handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayCreate(const mx_uint *shape,
                              mx_uint ndim,
                              int dev_mask,
                              int dev_id,
                              int delay_alloc,
                              NDArrayHandle *out);
/*!
 * \brief create a NDArray handle that is loaded from raw bytes.
 * \param buf the head of the raw bytes
 * \param size size of the raw bytes
 * \param out the returning handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayLoadFromRawBytes(const void *buf,
                                        mx_ulong size,
                                        NDArrayHandle *out);
/*!
 * \brief save the NDArray into raw bytes.
 * \param handle the NDArray handle
 * \param out_size size of the raw bytes
 * \param out_buf the head of returning memory bytes.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArraySaveRawBytes(NDArrayHandle handle,
                                    mx_ulong *out_size,
                                    const char **out_buf);
/*!
 * \brief Save list of narray into the file.
 * \param fname name of the file.
 * \param num_args number of arguments to save.
 * \param args the array of NDArrayHandles to be saved.
 * \param keys the name of the NDArray, optional, can be NULL
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArraySave(const char* fname,
                            mx_uint num_args,
                            NDArrayHandle* args,
                            const char** keys);
/*!
 * \brief Load list of narray from the file.
 * \param fname name of the file.
 * \param out_size number of narray loaded.
 * \param out_arr head of the returning narray handles.
 * \param out_name_size size of output name arrray.
 * \param out_names the names of returning NDArrays, can be NULL
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayLoad(const char* fname,
                            mx_uint *out_size,
                            NDArrayHandle** out_arr,
                            mx_uint *out_name_size,
                            const char*** out_names);
/*!
 * \brief Perform a synchronize copy from a continugous CPU memory region.
 *
 *  This function will call WaitToWrite before the copy is performed.
 *  This is useful to copy data from existing memory region that are
 *  not wrapped by NDArray(thus dependency not being tracked).
 *
 * \param handle the NDArray handle
 * \param data the data source to copy from.
 * \param size the memory size we want to copy from.
 */
MXNET_DLL int MXNDArraySyncCopyFromCPU(NDArrayHandle handle,
                                       const mx_float *data,
                                       size_t size);
/*!
 * \brief Perform a synchronize copyto a continugous CPU memory region.
 *
 *  This function will call WaitToRead before the copy is performed.
 *  This is useful to copy data from existing memory region that are
 *  not wrapped by NDArray(thus dependency not being tracked).
 *
 * \param handle the NDArray handle
 * \param data the data source to copy into.
 * \param size the memory size we want to copy into.
 */
MXNET_DLL int MXNDArraySyncCopyToCPU(NDArrayHandle handle,
                                    mx_float *data,
                                    size_t size);
/*!
 * \brief Wait until all the pending writes with respect NDArray are finished.
 *  Always call this before read data out synchronizely.
 * \param handle the NDArray handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayWaitToRead(NDArrayHandle handle);
/*!
 * \brief Wait until all the pending read/write with respect NDArray are finished.
 *  Always call this before write data into NDArray synchronizely.
 * \param handle the NDArray handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayWaitToWrite(NDArrayHandle handle);
/*!
 * \brief wait until all delayed operations in
 *   the system is completed
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayWaitAll();
/*!
 * \brief free the narray handle
 * \param handle the handle to be freed
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayFree(NDArrayHandle handle);
/*!
 * \brief get the shape of the array
 * \param handle the handle to the narray
 * \param out_dim the output dimension
 * \param out_pdata pointer holder to get data pointer of the shape
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayGetShape(NDArrayHandle handle,
                               mx_uint *out_dim,
                               const mx_uint **out_pdata);
/*!
 * \brief get the content of the data in NDArray
 * \param handle the handle to the narray
 * \param out_pdata pointer holder to get pointer of data
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayGetData(NDArrayHandle handle,
                              mx_float **out_pdata);
/*!
 * \brief get the context of the NDArray
 * \param handle the handle to the narray
 * \param out_dev_mask the output device mask
 * \param out_dev_id the output device id
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXNDArrayGetContext(NDArrayHandle handle,
                                 int *out_dev_mask,
                                 int *out_dev_id);

//--------------------------------
// Part 2: functions on NDArray
//--------------------------------
/*!
 * \brief list all the available functions handles
 *   most user can use it to list all the needed functions
 * \param out_size the size of returned array
 * \param out_array the output function array
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXListFunctions(mx_uint *out_size,
                              FunctionHandle **out_array);
/*!
 * \brief get the function handle by name
 * \param name the name of the function
 * \param out the corresponding function handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXGetFunction(const char *name,
                            FunctionHandle *out);
/*!
 * \brief Get the information of the function handle.
 * \param fun The function handle.
 * \param name The returned name of the function.
 * \param description The returned description of the function.
 * \param num_args Number of arguments.
 * \param arg_names Name of the arguments.
 * \param arg_type_infos Type informations about the arguments.
 * \param arg_descriptions Description information about the arguments.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXFuncGetInfo(FunctionHandle fun,
                            const char **name,
                            const char **description,
                            mx_uint *num_args,
                            const char ***arg_names,
                            const char ***arg_type_infos,
                            const char ***arg_descriptions);
/*!
 * \brief get the argument requirements of the function
 * \param fun input function handle
 * \param num_use_vars how many NDArrays to be passed in as used_vars
 * \param num_scalars scalar variable is needed
 * \param num_mutate_vars how many NDArrays to be passed in as mutate_vars
 * \param type_mask the type mask of this function
 * \return 0 when success, -1 when failure happens
 * \sa MXFuncInvoke
 */
MXNET_DLL int MXFuncDescribe(FunctionHandle fun,
                             mx_uint *num_use_vars,
                             mx_uint *num_scalars,
                             mx_uint *num_mutate_vars,
                             int *type_mask);
/*!
 * \brief invoke a function, the array size of passed in arguments
 *   must match the values in the
 * \param fun the function
 * \param use_vars the normal arguments passed to function
 * \param scalar_args the scalar qarguments
 * \param mutate_vars the mutate arguments
 * \return 0 when success, -1 when failure happens
 * \sa MXFuncDescribeArgs
 */
MXNET_DLL int MXFuncInvoke(FunctionHandle fun,
                           NDArrayHandle *use_vars,
                           mx_float *scalar_args,
                           NDArrayHandle *mutate_vars);

//--------------------------------------------
// Part 3: symbolic configuration generation
//--------------------------------------------
/*!
 * \brief list all the available AtomicSymbolEntry
 * \param out_size the size of returned array
 * \param out_array the output AtomicSymbolCreator array
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolListAtomicSymbolCreators(mx_uint *out_size,
                                               AtomicSymbolCreator **out_array);
/*!
 * \brief Get the detailed information about atomic symbol.
 * \param creator the AtomicSymbolCreator.
 * \param name The returned name of the creator.
 * \param description The returned description of the symbol.
 * \param num_args Number of arguments.
 * \param arg_names Name of the arguments.
 * \param arg_type_infos Type informations about the arguments.
 * \param arg_descriptions Description information about the arguments.
 * \param key_var_num_args The keyword argument for specifying variable number of arguments.
 *            When this parameter has non-zero length, the function allows variable number
 *            of positional arguments, and will need the caller to pass it in in
 *            MXSymbolCreateAtomicSymbol,
 *            With key = key_var_num_args, and value = number of positional arguments.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolGetAtomicSymbolInfo(AtomicSymbolCreator creator,
                                          const char **name,
                                          const char **description,
                                          mx_uint *num_args,
                                          const char ***arg_names,
                                          const char ***arg_type_infos,
                                          const char ***arg_descriptions,
                                          const char **key_var_num_args);
/*!
 * \brief Create an AtomicSymbol.
 * \param creator the AtomicSymbolCreator
 * \param num_param the number of parameters
 * \param keys the keys to the params
 * \param vals the vals of the params
 * \param out pointer to the created symbol handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolCreateAtomicSymbol(AtomicSymbolCreator creator,
                                         int num_param,
                                         const char **keys,
                                         const char **vals,
                                         SymbolHandle *out);
/*!
 * \brief Create a Variable Symbol.
 * \param name name of the variable
 * \param out pointer to the created symbol handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolCreateVariable(const char *name, SymbolHandle *out);
/*!
 * \brief Create a Symbol by grouping list of symbols together
 * \param num_symbols number of symbols to be grouped
 * \param symbols array of symbol handles
 * \param out pointer to the created symbol handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolCreateGroup(mx_uint num_symbols,
                                  SymbolHandle *symbols,
                                  SymbolHandle *out);
/*!
 * \brief Load a symbol from a json file.
 * \param fname the file name.
 * \param out the output symbol.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolCreateFromFile(const char *fname, SymbolHandle *out);
/*!
 * \brief Load a symbol from a json string.
 * \param json the json string.
 * \param out the output symbol.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolCreateFromJSON(const char *json, SymbolHandle *out);
/*!
 * \brief Save a symbol into a json file.
 * \param sym the input symbol.
 * \param fname the file name.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolSaveToFile(SymbolHandle symbol, const char *fname);
/*!
 * \brief Save a symbol into a json string
 * \param sym the input symbol.
 * \param out_json output json string.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolSaveToJSON(SymbolHandle symbol, const char **out_json);
/*!
 * \brief Free the symbol handle.
 * \param symbol the symbol
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolFree(SymbolHandle symbol);
/*!
 * \brief Copy the symbol to another handle
 * \param symbol the source symbol
 * \param out used to hold the result of copy
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolCopy(SymbolHandle symbol, SymbolHandle *out);
/*!
 * \brief Print the content of symbol, used for debug.
 * \param symbol the symbol
 * \param out_str pointer to hold the output string of the printing.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolPrint(SymbolHandle symbol, const char **out_str);
/*!
 * \brief List arguments in the symbol.
 * \param symbol the symbol
 * \param out_size output size
 * \param out_str_array pointer to hold the output string array
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolListArguments(SymbolHandle symbol,
                                    mx_uint *out_size,
                                    const char ***out_str_array);
/*!
 * \brief List returns in the symbol.
 * \param symbol the symbol
 * \param out_size output size
 * \param out_str_array pointer to hold the output string array
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolListOutputs(SymbolHandle symbol,
                                  mx_uint *out_size,
                                  const char ***out_str_array);
/*!
 * \brief List auxiliary states in the symbol.
 * \param symbol the symbol
 * \param out_size output size
 * \param out_str_array pointer to hold the output string array
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolListAuxiliaryStates(SymbolHandle symbol,
                                          mx_uint *out_size,
                                          const char ***out_str_array);
/*!
 * \brief Compose the symbol on other symbols.
 *
 *  This function will change the sym hanlde.
 *  To achieve function apply behavior, copy the symbol first
 *  before apply.
 *
 * \param sym the symbol to apply
 * \param name the name of symbol
 * \param num_args number of arguments
 * \param keys the key of keyword args (optional)
 * \param args arguments to sym
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolCompose(SymbolHandle sym,
                              const char *name,
                              mx_uint num_args,
                              const char** keys,
                              SymbolHandle* args);
/*!
 * \brief Get the gradient graph of the symbol
 *
 * \param sym the symbol to get gradient
 * \param num_wrt number of arguments to get gradient
 * \param wrt the name of the arguments to get gradient
 * \param out the returned symbol that has gradient
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolGrad(SymbolHandle sym,
                           mx_uint num_wrt,
                           const char** wrt,
                           SymbolHandle* out);
/*!
 * \brief infer shape of unknown input shapes given the known one.
 *  The shapes are packed into a CSR matrix represented by arg_ind_ptr and arg_shape_data
 *  The call will be treated as a kwargs call if key != nullptr or num_args==0, otherwise it is positional.
 *
 * \param sym symbol handle
 * \param num_args numbe of input arguments.
 * \param keys the key of keyword args (optional)
 * \param arg_ind_ptr the head pointer of the rows in CSR
 * \param arg_shape_data the content of the CSR
 * \param in_shape_size sizeof the returning array of in_shapes
 * \param in_shape_ndim returning array of shape dimensions of eachs input shape.
 * \param in_shape_data returning array of pointers to head of the input shape.
 * \param out_shape_size sizeof the returning array of out_shapes
 * \param out_shape_ndim returning array of shape dimensions of eachs input shape.
 * \param out_shape_data returning array of pointers to head of the input shape.
 * \param aux_shape_size sizeof the returning array of aux_shapes
 * \param aux_shape_ndim returning array of shape dimensions of eachs auxiliary shape.
 * \param aux_shape_data returning array of pointers to head of the auxiliary shape.
 * \param complete whether infer shape completes or more information is needed.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXSymbolInferShape(SymbolHandle sym,
                                 mx_uint num_args,
                                 const char** keys,
                                 const mx_uint *arg_ind_ptr,
                                 const mx_uint *arg_shape_data,
                                 mx_uint *in_shape_size,
                                 const mx_uint **in_shape_ndim,
                                 const mx_uint ***in_shape_data,
                                 mx_uint *out_shape_size,
                                 const mx_uint **out_shape_ndim,
                                 const mx_uint ***out_shape_data,
                                 mx_uint *aux_shape_size,
                                 const mx_uint **aux_shape_ndim,
                                 const mx_uint ***aux_shape_data,
                                 int *complete);
//--------------------------------------------
// Part 4: Executor interface
//--------------------------------------------
/*!
 * \brief Print the content of execution plan, used for debug.
 * \param handle the executor.
 * \param out_str pointer to hold the output string of the printing.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXExecutorPrint(ExecutorHandle symbol, const char **out_str);
/*!
 * \brief Executor forward method
 *
 * \param handle executor handle
 * \param is_train bool value to indicate whether the forward pass is for evaluation
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXExecutorForward(ExecutorHandle handle, bool is_train);
/*!
 * \brief Excecutor run backward
 *
 * \param handle execute handle
 * \param len lenth
 * \param head_grads NDArray handle for heads' gradient
 *
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXExecutorBackward(ExecutorHandle handle,
                                 mx_uint len,
                                 NDArrayHandle *head_grads);

/*!
 * \brief Get executor's head NDArray
 *
 * \param handle executor handle
 * \param out_size output narray vector size
 * \param out out put narray handles
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXExecutorOutputs(ExecutorHandle handle,
                                mx_uint *out_size,
                                NDArrayHandle **out);

/*!
 * \brief Generate Executor from symbol
 *
 * \param symbol_handle symbol handle
 * \param dev_mask device mask
 * \param dev_id device id
 * \param len length
 * \param in_args in args array
 * \param arg_grad_store arg grads handle array
 * \param grad_req_type grad req array
 * \param aux_states_len length of auxiliary states
 * \param aux_states auxiliary states array
 * \param out output executor handle
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXExecutorBind(SymbolHandle symbol_handle,
                             int dev_mask,
                             int dev_id,
                             mx_uint len,
                             NDArrayHandle *in_args,
                             NDArrayHandle *arg_grad_store,
                             mx_uint *grad_req_type,
                             mx_uint aux_states_len,
                             NDArrayHandle *aux_states,
                             ExecutorHandle *out);

//--------------------------------------------
// Part 5: IO Interface
//--------------------------------------------
/*!
 * \brief List all the available iterator entries
 * \param out_size the size of returned iterators
 * \param out_array the output iteratos entries
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXListDataIters(mx_uint *out_size,
                              DataIterCreator **out_array);
/*!
 * \brief Init an iterator, init with parameters
 * the array size of passed in arguments
 * \param handle of the iterator creator
 * \param num_param number of parameter
 * \param keys parameter keys
 * \param vals parameter values
 * \param out resulting iterator
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXDataIterCreateIter(DataIterCreator handle,
                                   int num_param,
                                   const char **keys,
                                   const char **vals,
                                   DataIterHandle *out);
/*!
 * \brief Get the detailed information about data iterator.
 * \param creator the DataIterCreator.
 * \param name The returned name of the creator.
 * \param description The returned description of the symbol.
 * \param num_args Number of arguments.
 * \param arg_names Name of the arguments.
 * \param arg_type_infos Type informations about the arguments.
 * \param arg_descriptions Description information about the arguments.
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXDataIterGetIterInfo(AtomicSymbolCreator creator,
                                    const char **name,
                                    const char **description,
                                    mx_uint *num_args,
                                    const char ***arg_names,
                                    const char ***arg_type_infos,
                                    const char ***arg_descriptions);
/*!
 * \brief Free the handle to the IO module
 * \param handle the handle pointer to the data iterator
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXDataIterFree(DataIterHandle handle);
/*!
 * \brief Move iterator to next position
 * \param handle the handle to iterator
 * \param out return value of next
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXDataIterNext(DataIterHandle handle,
                             int *out);
/*!
 * \brief Call iterator.Reset
 * \param handle the handle to iterator
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXDataIterBeforeFirst(DataIterHandle handle);

/*!
 * \brief Get the handle to the NDArray of underlying data
 * \param handle the handle pointer to the data iterator
 * \param out handle to underlying data NDArray
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXDataIterGetData(DataIterHandle handle,
                                NDArrayHandle *out);
/*!
 * \brief Get the handle to the NDArray of underlying label
 * \param handle the handle pointer to the data iterator
 * \param out the handle to underlying label NDArray
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXDataIterGetLabel(DataIterHandle handle,
                                 NDArrayHandle *out);
//--------------------------------------------
// Part 5: KVStore interface
//--------------------------------------------
/*!
 * \brief start the kvstore
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXKVStoreStart();
/*!
 * \brief stop the kvstore
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXKVStoreStop();

/*!
 * \brief Init a list of (key,value) pairs in kvstore
 * \param num the number of key-value pairs
 * \param keys the list of keys
 * \param vals the list of values
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXKVStoreInit(int num,
                            int* keys,
                            NDArrayHandle* vals);

/*!
 * \brief Push a list of (key,value) pairs to kvstore
 * \param num the number of key-value pairs
 * \param keys the list of keys
 * \param vals the list of values
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXKVStorePush(int num,
                            int* keys,
                            NDArrayHandle* vals);


/*!
 * \brief pull a list of (key, value) pairs from the kvstore
 * \param num the number of key-value pairs
 * \param keys the list of keys
 * \param vals the list of values
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXKVStorePull(int num,
                            int* keys,
                            NDArrayHandle* vals);

/*!
 * \brief user-defined updater for the kvstore
 * It's this updater's responsibility to delete \a recv and \a local
 * \param the key
 * \param recv the pushed value on this key
 * \param local the value stored on local on this key
 */
typedef void (MXKVStoreUpdater)(int key, NDArrayHandle recv, NDArrayHandle local);

/*!
 * \brief register an push updater
 * \param updater udpater function
 * \return 0 when success, -1 when failure happens
 */
MXNET_DLL int MXKVStoreSetUpdater(MXKVStoreUpdater updater);

#endif  // MXNET_C_API_H_