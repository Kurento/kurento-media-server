#ifndef JOINABLE_IMPL
#define JOINABLE_IMPL

#include "joinable_types.h"
#include "types/MediaObjectImpl.h"

using ::com::kurento::kms::api::Joinable;
using ::com::kurento::kms::api::Direction;

namespace com { namespace kurento { namespace kms {

class JoinableImpl : public virtual Joinable, public virtual MediaObjectImpl {
public:
	JoinableImpl();
	~JoinableImpl() throw() {};

	void join(const JoinableImpl& to, const Direction::type direction);
	void unjoin(const JoinableImpl& to);
	std::vector<Joinable> &getJoinees();
	std::vector<Joinable> &getDirectionJoiness(const Direction::type direction);
};

}}} // com::kurento::kms

#endif /* JOINABLE_IMPL */