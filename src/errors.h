enum errors {
  GDB_NO_ERROR = CEXCEPT_NO_ERROR,

  /* Any generic error, the corresponding text is in
     exception.message.  */
  GENERIC_ERROR,

  /* Something requested was not found.  */
  NOT_FOUND_ERROR,

  /* Thread library lacks support necessary for finding thread local
     storage.  */
  TLS_NO_LIBRARY_SUPPORT_ERROR,

  /* Load module not found while attempting to find thread local
     storage.  */
  TLS_LOAD_MODULE_NOT_FOUND_ERROR,

  /* Thread local storage has not been allocated yet.  */
  TLS_NOT_ALLOCATED_YET_ERROR,

  /* Something else went wrong while attempting to find thread local
     storage.  The ``struct cexception'' message field provides more
     detail.  */
  TLS_GENERIC_ERROR,

  /* Problem parsing an XML document.  */
  XML_PARSE_ERROR,

  /* Error accessing memory.  */
  MEMORY_ERROR,

  /* Feature is not supported in this copy of GDB.  */
  UNSUPPORTED_ERROR,

  /* Value not available.  E.g., a register was not collected in a
     traceframe.  */
  NOT_AVAILABLE_ERROR,

  /* DW_OP_GNU_entry_value resolving failed.  */
  NO_ENTRY_VALUE_ERROR,

  /* Add more errors here.  */
  NR_ERRORS
};
