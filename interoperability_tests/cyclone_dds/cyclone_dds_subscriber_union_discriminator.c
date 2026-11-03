#include <float.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ddsc/dds.h"
#include "UnionDiscriminator.h"

#define MAX_SAMPLES 1

int main()
{
	const char *topic_name = "UnionDiscriminator";

	const dds_entity_t participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL /*qos*/, NULL /*listener*/);
	if (participant < 0)
	{
		DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
	}
	const dds_entity_t topic = dds_create_topic(participant, &interoperability_test_Message_desc, topic_name, NULL /*qos*/, NULL /*listener*/);
	if (topic < 0)
	{
		DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
	}
	dds_qos_t *qos = dds_create_qos();
	dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(1));
	dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);

	const dds_entity_t data_reader = dds_create_reader(participant, topic, qos, NULL /*listener*/);
	if (data_reader < 0)
	{
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-data_reader));
	}

	dds_return_t rc;

	rc = dds_set_status_mask(data_reader, DDS_SUBSCRIPTION_MATCHED_STATUS);
	if (rc != DDS_RETCODE_OK)
	{
		DDS_FATAL("dds_set_status_mask: %s\n", dds_strretcode(-rc));
	}

	dds_entity_t waitset = dds_create_waitset(participant);

	rc = dds_waitset_attach(waitset, data_reader, data_reader);
	if (rc != DDS_RETCODE_OK)
	{
		DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-rc));
	}

	dds_attach_t wsresults[1];
	const dds_return_t wsresultsize = 1U;
	rc = dds_waitset_wait(waitset, wsresults, wsresultsize, DDS_SECS(30));
	if (rc == 0)
	{
		DDS_FATAL("dds_waitset_wait: timeout: Subscription not matched");
	}
	if (rc != wsresultsize)
	{
		DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-rc));
	}

	rc = dds_set_status_mask(data_reader, DDS_DATA_AVAILABLE_STATUS);
	if (rc != DDS_RETCODE_OK)
	{
		DDS_FATAL("dds_set_status_mask: %s\n", dds_strretcode(-rc));
	}
	rc = dds_waitset_wait(waitset, wsresults, wsresultsize, DDS_SECS(30));
	if (rc == 0)
	{
		DDS_FATAL("dds_waitset_wait: timeout: No data received");
	}
	if (rc != wsresultsize)
	{
		DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-rc));
	}

	void *samples[MAX_SAMPLES];
	dds_sample_info_t infos[MAX_SAMPLES];
	samples[0] = interoperability_test_Message__alloc();

	rc = dds_read(data_reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
	if (rc < 0)
	{
		DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));
	}

	if ((rc > 0) && (infos[0].valid_data))
	{
		const interoperability_test_Message *msg = (interoperability_test_Message *)samples[0];
        const interoperability_test_Vec3F32 *velocity = &msg->payload._u.velocity;
		printf("Received: %s { id: %u, payload: { velocity: { x: %f, y: %f, z: %f } } }\n",
           interoperability_test_Message_desc.m_typename, msg->id, velocity->x, velocity->y, velocity->z);
		assert(msg->id == 1);
		assert(msg->payload._d == interoperability_test_MESSAGE_PAYLOAD_DISCRIMINATOR_VELOCITY);
        assert(msg->payload._u.velocity.x == 1.0);
        assert(msg->payload._u.velocity.y == 2.0);
        assert(msg->payload._u.velocity.z == 3.0);
	}

	// Sleep to allow sending acknowledgements
	dds_sleepfor(DDS_SECS(2));
}