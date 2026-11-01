#include "doctest.h"

#include <stdexcept>
#include <string>
#include <type_traits>

#include "engine/NotificationService.h"
#include "engine/EmailAdapter.h"
#include "engine/ExternalEmailService.h"
#include "engine/Participant.h"

using namespace tividy;

TEST_CASE("EmailAdapter implements NotificationService")
{
    CHECK(
        std::is_base_of<
            NotificationService,
            EmailAdapter
        >::value
    );
}

TEST_CASE("EmailAdapter starts without an external service")
{
    EmailAdapter adapter;

    CHECK(adapter.service == nullptr);
}

TEST_CASE("EmailAdapter forwards notifications to ExternalEmailService")
{
    ExternalEmailService service;

    EmailAdapter adapter;
    adapter.service = &service;

    Participant recipient("Student");

    CHECK_NOTHROW(
        adapter.sendNotification(
            &recipient,
            "Your complaint has been updated."
        )
    );
}

TEST_CASE("EmailAdapter rejects notification without a configured service")
{
    EmailAdapter adapter;

    Participant recipient("Student");

    CHECK_THROWS_AS(
        adapter.sendNotification(
            &recipient,
            "Your complaint has been updated."
        ),
        std::logic_error
    );
}

TEST_CASE("EmailAdapter rejects a null recipient")
{
    ExternalEmailService service;

    EmailAdapter adapter;
    adapter.service = &service;

    CHECK_THROWS_AS(
        adapter.sendNotification(
            nullptr,
            "Your complaint has been updated."
        ),
        std::invalid_argument
    );
}