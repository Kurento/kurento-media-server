#ifndef JOINABLE_IMPL
#define JOINABLE_IMPL

#include "joinable_types.h"
#include "types/MediaObjectImpl.h"

using ::com::kurento::kms::api::Joinable;
using ::com::kurento::kms::api::Direction;
using ::com::kurento::kms::api::StreamType;

namespace com { namespace kurento { namespace kms {

class JoinableImpl : public Joinable, public virtual MediaObjectImpl {
public:
	JoinableImpl();
	~JoinableImpl() throw() {};

	std::vector<StreamType::type> getStreams(const Joinable& joinable);

	void join(const JoinableImpl& to, const Direction::type direction);
	void unjoin(const JoinableImpl& to);

	void join(const JoinableImpl& to, const StreamType::type stream, const Direction::type direction);
	void unjoin(const JoinableImpl& to, const StreamType::type stream);

	std::vector<Joinable> &getJoinees();
	std::vector<Joinable> &getDirectionJoiness(const Direction::type direction);

	std::vector<Joinable> &getJoinees(const StreamType::type stream);
	std::vector<Joinable> &getDirectionJoiness(const StreamType::type stream, const Direction::type direction);
};

}}} // com::kurento::kms

#endif /* JOINABLE_IMPL */
