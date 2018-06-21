#pragma once

#include <functional>
#include <set>
#include <shared_mutex>

#include "envoy/runtime/runtime.h"
#include "envoy/secret/secret_manager.h"
#include "envoy/ssl/context_manager.h"

namespace Envoy {
namespace Ssl {

/**
 * The SSL context manager has the following threading model:
 * Contexts can be allocated via any thread (through in practice they are only allocated on the main
 * thread). They can be released from any thread (and in practice are since cluster information can
 * be released from any thread). Context allocation/free is a very uncommon thing so we just do a
 * global lock to protect it all.
 */
class ContextManagerImpl final : public ContextManager {
public:
  ContextManagerImpl(Runtime::Loader& runtime, Secret::SecretManager& secret_manager)
      : runtime_(runtime), secret_manager_(secret_manager) {}
  ~ContextManagerImpl();

  /**
   * Allocated contexts are owned by the caller. However, we need to be able to iterate them for
   * admin purposes. When a caller frees a context it will tell us to release it also from the list
   * of contexts.
   */
  void releaseContext(Context* context);

  // Ssl::ContextManager
  Ssl::ClientContextPtr createSslClientContext(Stats::Scope& scope,
                                               const ClientContextConfig& config) override;
  Ssl::ClientContextPtr updateSslClientContext(const Ssl::ClientContextPtr& context,
                                               Stats::Scope& scope,
                                               const ClientContextConfig& config) override;

  Ssl::ServerContextPtr
  createSslServerContext(Stats::Scope& scope, const ServerContextConfig& config,
                         const std::vector<std::string>& server_names) override;
  virtual ServerContextPtr
  updateSslServerContext(const ServerContextPtr& context, Stats::Scope& scope,
                         const ServerContextConfig& config,
                         const std::vector<std::string>& server_names) override;

  size_t daysUntilFirstCertExpires() const override;
  void iterateContexts(std::function<void(const Context&)> callback) override;

  Secret::SecretManager& secretManager() override { return secret_manager_; }

private:
  Runtime::Loader& runtime_;
  std::set<Context*> contexts_;
  mutable std::shared_timed_mutex contexts_lock_;
  Secret::SecretManager& secret_manager_;
};

} // namespace Ssl
} // namespace Envoy
