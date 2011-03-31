#include "IndexManager.h"

#include <common/SFLogger.h>
#include <ir/index_manager/utility/StringUtils.h>

#include <common/Utilities.h>

#include <boost/assert.hpp>

#include <stdexcept>

namespace sf1r
{
std::string IndexManager::DATE("DATE");
IndexManager::IndexManager()
{
}

IndexManager::~IndexManager()
{
}

namespace { // {anonymous}

struct PropertyValue2IndexPropertyType
: public boost::static_visitor<>
{

    PropertyValue2IndexPropertyType(PropertyType& out)
    : out_(out)
    {
    }

    template<typename T>
    void operator()(const T& value)
    {
        sflog->error(SFL_IDX, 70101);
        throw std::runtime_error("Type not supported in PropertyType");
    }
    void operator()(int64_t value)
    {
        out_ = static_cast<int64_t>(value);
    }
    void operator()(uint64_t value)
    {
        out_ = static_cast<int64_t>(value);
    }
    void operator()(float value)
    {
        out_ = value;
    }
    void operator()(double value)
    {
        out_ = value;
    }
    void operator()(const std::string& value)
    {
        izenelib::ir::indexmanager::trim(const_cast<std::string&>(value));
        out_ = izenelib::util::UString(value,izenelib::util::UString::UTF_8);
    }
    void operator()(const izenelib::util::UString& value)
    {
        izenelib::ir::indexmanager::trim(const_cast<izenelib::util::UString&>(value));
        out_ = value;
    }

private:
    PropertyType& out_;
};

} // namespace {anonymous}

void IndexManager::convertData(const std::string& property, const PropertyValue& in, PropertyType& out)
{
    PropertyValue2IndexPropertyType converter(out);

    if(property==DATE)
    {
        int64_t time = sf1r::Utilities::convertDate(in.get<std::string>());
        PropertyValue inValue(time);
        boost::apply_visitor(converter, inValue.getVariant());
    }
    else
        boost::apply_visitor(converter, in.getVariant());
}

void IndexManager::makeRangeQuery(QueryFiltering::FilteringOperation filterOperation, const std::string& property, 
	   const std::vector<PropertyValue>& filterParam, boost::shared_ptr<BitVector> docIdSet)
{
    collectionid_t colId = 1;
    std::string propertyL = sf1r::Utilities::toUpper(property);
    switch (filterOperation)
    {
    case QueryFiltering::EQUAL:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL,filterParam[0], value);
        getDocsByPropertyValue(colId, property, value, *docIdSet);
    }
        break;
    case QueryFiltering::INCLUDE:
    {
        for(std::vector<PropertyValue>::const_iterator iter = filterParam.begin(); iter != filterParam.end(); ++iter)
        {
            PropertyType value;
            convertData(propertyL, *iter, value);
            getDocsByPropertyValue(colId, property, value, *docIdSet);
        }
    }
        break;
    case QueryFiltering::EXCLUDE:
    {
        for(std::vector<PropertyValue>::const_iterator iter = filterParam.begin(); iter != filterParam.end(); ++iter)
        {
            PropertyType value;
            convertData(propertyL, *iter, value);
            getDocsByPropertyValue(colId, property, value, *docIdSet);
        }
        docIdSet->toggle();
    }
        break;
     case QueryFiltering::NOT_EQUAL:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL, filterParam[0], value);
        getDocsByPropertyValueNotEqual(colId, property, value, *docIdSet);
    }
        break;
    case QueryFiltering::GREATER_THAN:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL, filterParam[0], value);
        getDocsByPropertyValueGreaterThan(colId, property, value, *docIdSet);
    }
        break;
    case QueryFiltering::GREATER_THAN_EQUAL:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL, filterParam[0], value);
        getDocsByPropertyValueGreaterThanOrEqual(colId, property, value, *docIdSet);
    }
        break;
    case QueryFiltering::LESS_THAN:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL, filterParam[0], value);
        getDocsByPropertyValueLessThan(colId, property, value, *docIdSet);
    }
        break;
    case QueryFiltering::LESS_THAN_EQUAL:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL, filterParam[0], value);
        getDocsByPropertyValueLessThanOrEqual(colId, property, value, *docIdSet);
    }
        break;
    case QueryFiltering::RANGE:
    {
        vector<docid_t> docs;
        PropertyType value1,value2;
        BOOST_ASSERT(filterParam.size() >= 2);
        convertData(propertyL, filterParam[0], value1);
        convertData(propertyL, filterParam[1], value2);

        getDocsByPropertyValueRange(colId, property, value1, value2, *docIdSet);
    }
        break;
    case QueryFiltering::PREFIX:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL, filterParam[0], value);
        getDocsByPropertyValueStart(colId, property, value, *docIdSet);
    }
        break;
    case QueryFiltering::SUFFIX:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL, filterParam[0], value);
        getDocsByPropertyValueEnd(colId, property, value, *docIdSet);
    }
        break;
    case QueryFiltering::SUB_STRING:
    {
        PropertyType value;
        BOOST_ASSERT(!filterParam.empty());
        convertData(propertyL, filterParam[0], value);
        getDocsByPropertyValueSubString(colId, property, value, *docIdSet);
    }
        break;
    default:
        break;
    }
}

}
