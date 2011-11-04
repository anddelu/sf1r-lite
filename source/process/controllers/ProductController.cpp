#include <bundles/product/ProductSearchService.h>
#include "ProductController.h"
#include <product-manager/product_manager.h>
#include <common/Keys.h>
#include <util/ustring/UString.h>
#include <vector>
#include <string>
#include <boost/unordered_set.hpp>

namespace sf1r
{

using driver::Keys;
using namespace izenelib::driver;

bool ProductController::check_product_manager_()
{
    if(collectionHandler_->productSearchService_==NULL)
    {
        response().addError("ProductManager not enabled.");
        return false;
    }
    product_manager_ = collectionHandler_->productSearchService_->GetProductManager();

    if(!product_manager_)
    {
        response().addError("ProductManager not enabled.");
        return false;
    }
    return true;
}

bool ProductController::require_docs_()
{
    Value& resources = request()[Keys::docid_list];
    for(uint32_t i=0;i<resources.size();i++)
    {
        Value& resource = resources(i);
        uint32_t docid = asUint(resource);
        docid_list_.push_back(docid);
    }
    if (docid_list_.empty())
    {
        response().addError("Require docid_list in request.");
        return false;
    }
    return true;
}

bool ProductController::require_uuid_()
{
    Value& vuuid = request()[Keys::uuid];
    std::string suuid = asString(vuuid);
    if(suuid.empty())
    {
        response().addError("Require uuid in request.");
        return false;
    }
    else
    {
        uuid_ = izenelib::util::UString(suuid, izenelib::util::UString::UTF_8);
    }
    return true;
}

bool ProductController::require_doc_()
{
    doc_.clear();
    Value& resource = request()[Keys::resource];
    const Value::ObjectType& objectValue = resource.getObject();
    
    for (Value::ObjectType::const_iterator it = objectValue.begin();
         it != objectValue.end(); ++it)
    {
        std::string pname = it->first;
        izenelib::util::UString pvalue( asString(it->second), izenelib::util::UString::UTF_8 );
        doc_.property(pname) = pvalue;
    }
    const PMConfig& config = product_manager_->GetConfig();
    //check validation
    if(!doc_.hasProperty(config.docid_property_name))
    {
        response().addError("Require DOCID property in request.");
        return false;
    }
    if(doc_.hasProperty(config.price_property_name))
    {
        response().addError("Can not update Price property manually.");
        return false;
    }
    if(doc_.hasProperty(config.itemcount_property_name))
    {
        response().addError("Can not update itemcount property manually.");
        return false;
    }
    return true;
}


/**
 * @brief Action \b add_new_group. Use new documents id list to generate a new group
 *
 * @section request
 *
 * - @b collection* (@c String): Collection name.
 * - @b docid_list* (@c Array): document id list
 *
 * @section response
 *
 * - @b uuid (@c String): Generated uuid.
 * 
 *
 * @section Example
 *
 * Request
 * @code
 * {
 *   "collection" : "b5mm",
 *   "docid_list": [5,6,7,8]
 * }
 * @endcode
 *
 * Response
 * @code
 * {
 *   "header": {"success": true},
 *   "uuid" : "xxx-yyy-zzz"
 * }
 * @endcode
 */
void ProductController::add_new_group()
{
    IZENELIB_DRIVER_BEFORE_HOOK(check_product_manager_());
    IZENELIB_DRIVER_BEFORE_HOOK(require_docs_());
    izenelib::util::UString uuid;
    if(!product_manager_->AddGroup(docid_list_, uuid))
    {
        response().addError(product_manager_->GetLastError());
        return;
    }
    std::string suuid;
    uuid.convertString(suuid, izenelib::util::UString::UTF_8);
    response()[Keys::uuid] = suuid;
}

/**
 * @brief Action \b append_to_group. Append new documents id list to an exist group
 *
 * @section request
 *
 * - @b collection* (@c String): Collection name.
 * - @b uuid*       (@c String): uuid
 * - @b docid_list* (@c Array): document id list
 *
 * @section response
 *
 * 
 *
 * @section Example
 *
 * Request
 * @code
 * {
 *   "collection" : "b5mm",
 *   "uuid" : "xxx-yyy-zzz",
 *   "docid_list": [5,6,7,8]
 * }
 * @endcode
 *
 * Response
 * @code
 * {
 *   "header": {"success": true},
 * }
 * @endcode
 */
void ProductController::append_to_group()
{
    IZENELIB_DRIVER_BEFORE_HOOK(check_product_manager_());
    IZENELIB_DRIVER_BEFORE_HOOK(require_uuid_());
    IZENELIB_DRIVER_BEFORE_HOOK(require_docs_());
    if(!product_manager_->AppendToGroup(uuid_, docid_list_))
    {
        response().addError(product_manager_->GetLastError());
        return;
    }
}

/**
 * @brief Action \b remove_from_group. Remove documents id list from an exist group
 *
 * @section request
 *
 * - @b collection* (@c String): Collection name.
 * - @b uuid*       (@c String): uuid
 * - @b docid_list* (@c Array): document id list
 *
 * @section response
 *
 * 
 *
 * @section Example
 *
 * Request
 * @code
 * {
 *   "collection" : "b5mm",
 *   "uuid" : "xxx-yyy-zzz",
 *   "docid_list": [5,6,7,8]
 * }
 * @endcode
 *
 * Response
 * @code
 * {
 *   "header": {"success": true},
 * }
 * @endcode
 */
void ProductController::remove_from_group()
{
    IZENELIB_DRIVER_BEFORE_HOOK(check_product_manager_());
    IZENELIB_DRIVER_BEFORE_HOOK(require_uuid_());
    IZENELIB_DRIVER_BEFORE_HOOK(require_docs_());
    if(!product_manager_->RemoveFromGroup(uuid_, docid_list_))
    {
        response().addError(product_manager_->GetLastError());
        return;
    }
}

/**
 * @brief Action \b recover. Recover product-manager data from backup
 *
 * @section request
 *
 * - @b collection* (@c String): Collection name.
 *
 * @section response
 *
 * 
 *
 * @section Example
 *
 * Request
 * @code
 * {
 *   "collection" : "b5mm",
 * }
 * @endcode
 *
 * Response
 * @code
 * {
 *   "header": {"success": true},
 * }
 * @endcode
 */
void ProductController::recover()
{
    IZENELIB_DRIVER_BEFORE_HOOK(check_product_manager_());
    if(!product_manager_->Recover())
    {
        response().addError(product_manager_->GetLastError());
        return;
    }
}

/**
 * @brief Action @b update_a_doc. Updates documents for A collection for product-manager
 *
 * @section request
 *
 * - @b collection* (@c String): Collection name for 'M'(b5mm).
 * - @b resource* (@c Object): A document resource in 'A' collection. Property key name is used as
 *   key. The corresponding value is the content of the property. Property @b
 *   DOCID is required, which is a unique document identifier specified by
 *   client. This DOCID is always the uuid in 'M' collection.
 *
 * @section response
 *
 * No extra fields.
 *
 * @section example
 *
 * @code
 * {
 *   "resource": {
 *     "DOCID": "xxx-yyy-zzz",
 *     "Title": "Hello, World (revision)",
 *     "Content": "This is a revised test post."
 *   }
 * }
 * @endcode
 */
void ProductController::update_a_doc()
{
    IZENELIB_DRIVER_BEFORE_HOOK(check_product_manager_());
    IZENELIB_DRIVER_BEFORE_HOOK(require_doc_());
    if(!product_manager_->UpdateADoc(doc_))
    {
        response().addError(product_manager_->GetLastError());
        return;
    }
}

    
}