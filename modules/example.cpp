#include <Filter.hpp>
#include <module.hpp>

#define TYPE_NAME "ExampleModuleFilter"

namespace kurento
{

class ExampleFilter : public Filter
{
public:
  ExampleFilter (MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
                 const std::map<std::string, KmsMediaParam> &params = emptyParams);
  virtual ~ExampleFilter() throw ();
};

ExampleFilter::ExampleFilter (MediaSet &mediaSet,
                              std::shared_ptr< MediaPipeline > parent,
                              const std::map< std::string, KmsMediaParam > &params)
  : Filter (mediaSet, parent, TYPE_NAME, params)
{
}

ExampleFilter::~ExampleFilter()
{
}

static std::shared_ptr<MediaObjectImpl>
object_factory (MediaSet &mediaSet, std::shared_ptr< MediaPipeline > parent,
                const std::map< std::string, KmsMediaParam > &params)
{
  std::shared_ptr<MediaObjectImpl> obj (new ExampleFilter (mediaSet, parent,
                                        params) );
  return obj;
}

static std::string
get_name()
{
  return TYPE_NAME;
}

KURENTO_CREATE_MODULE (KURENTO_MODULE_ELEMENT, get_name, object_factory);

} // kurento
