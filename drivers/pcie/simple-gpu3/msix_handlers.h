#ifndef MSIX_HANDLERS_H
#define MSIX_HANDLERS_H

#include <linux/irqreturn.h>

#define IntTypeList(_)                                                         \
  _(job_complete) _(pagefault) _(dma_read_complete) _(dma_write_complete)

#define ENUM_ITEM(_) _,

typedef enum IntType { IntTypeList(ENUM_ITEM) } IntType;

#define ENUM_HANDLER_DECLARE(_) irqreturn_t on_##_(int irq, void *dev_id);

IntTypeList(ENUM_HANDLER_DECLARE)

#define SGPU_CONCAT(a, b) a##b

#define ENUM_GET_HANDLER_PTR(_) &SGPU_CONCAT(on_, _),

typedef irqreturn_t (*irq_handler_t)(int, void *);

static const irq_handler_t all_handlers[] = {IntTypeList(ENUM_GET_HANDLER_PTR)};

#endif
