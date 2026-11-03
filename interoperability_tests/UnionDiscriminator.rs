pub mod interoperability {
    pub mod test {
        #[derive(
            ::core::fmt::Debug,
            ::core::clone::Clone,
            ::dust_dds::infrastructure::type_support::DdsType,
        )]
        #[dust_dds(extensibility = "final", name = "interoperability::test::Vec3F32")]
        pub struct Vec3F32 {
            pub x: f32,
            pub y: f32,
            pub z: f32,
        }

        #[derive(
            ::core::fmt::Debug,
            ::core::clone::Clone,
            ::dust_dds::infrastructure::type_support::DdsType,
        )]
        #[dust_dds(
            bit_bound = "8",
            extensibility = "final",
            name = "interoperability::test::MessagePayloadDiscriminator"
        )]
        #[allow(clippy::upper_case_acronyms, nonstandard_style)]
        pub enum MessagePayloadDiscriminator {
            MESSAGE_PAYLOAD_DISCRIMINATOR_POSITION = 0,
            MESSAGE_PAYLOAD_DISCRIMINATOR_VELOCITY,
            MESSAGE_PAYLOAD_DISCRIMINATOR_HEARTBEAT,
            MESSAGE_PAYLOAD_DISCRIMINATOR_ACKNOWLEDGEMENT,
        }

        #[derive(
            ::core::fmt::Debug,
            ::core::clone::Clone,
            ::dust_dds::infrastructure::type_support::DdsType,
        )]
        #[dust_dds(
            switch(MessagePayloadDiscriminator),
            extensibility = "final",
            name = "interoperability::test::MessagePayload"
        )]
        #[allow(non_camel_case_types)]
        pub enum MessagePayload {
            #[dust_dds(case = MessagePayloadDiscriminator::MESSAGE_PAYLOAD_DISCRIMINATOR_POSITION)]
            position(Vec3F32),
            #[dust_dds(case = MessagePayloadDiscriminator::MESSAGE_PAYLOAD_DISCRIMINATOR_VELOCITY)]
            velocity(Vec3F32),
            #[dust_dds(case = MessagePayloadDiscriminator::MESSAGE_PAYLOAD_DISCRIMINATOR_HEARTBEAT)]
            heartbeat,
            #[dust_dds(case = MessagePayloadDiscriminator::MESSAGE_PAYLOAD_DISCRIMINATOR_ACKNOWLEDGEMENT)]
            acknowledgement,
        }

        #[derive(
            ::core::fmt::Debug,
            ::core::clone::Clone,
            ::dust_dds::infrastructure::type_support::DdsType,
        )]
        #[dust_dds(extensibility = "final", name = "interoperability::test::Message")]
        pub struct Message {
            #[dust_dds(key)]
            pub id: u8,
            pub payload: MessagePayload,
        }
    }
}
