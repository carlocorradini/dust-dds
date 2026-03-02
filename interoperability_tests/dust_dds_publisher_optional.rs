use self::interoperability::test::OptionalType;
use dust_dds::{
    domain::domain_participant_factory::DomainParticipantFactory,
    infrastructure::{
        qos::{DataWriterQos, QosKind},
        qos_policy::{
            DurabilityQosPolicy, DurabilityQosPolicyKind, ReliabilityQosPolicy,
            ReliabilityQosPolicyKind,
        },
        status::{NO_STATUS, StatusKind},
        time::{Duration, DurationKind},
        type_support::TypeSupport,
    },
    listener::NO_LISTENER,
    wait_set::{Condition, WaitSet},
};

// TODO: remove when dust_dds_gen adds support for @optional
pub mod interoperability {
    pub mod test {
        use dust_dds::{
            infrastructure::type_support::TypeSupport,
            xtypes::{
                binding::XTypesBinding,
                data_storage::DataStorageMapping,
                dynamic_type::{
                    DynamicData, DynamicDataFactory, DynamicType, DynamicTypeBuilderFactory,
                    ExtensibilityKind, MemberDescriptor, TryConstructKind, TypeDescriptor,
                    TypeKind,
                },
            },
        };

        #[derive(Default, Debug, Clone, PartialEq)]
        pub struct OptionalType {
            pub maybe_a: Option<u32>,
            pub maybe_b: Option<f64>,
            pub maybe_c: Option<String>,
        }

        impl TypeSupport for OptionalType {
            fn get_type_name() -> &'static str {
                "interoperability::test::OptionalType"
            }

            fn get_type() -> DynamicType {
                let mut builder = DynamicTypeBuilderFactory::create_type(TypeDescriptor {
                    kind: TypeKind::STRUCTURE,
                    name: Self::get_type_name().to_string(),
                    base_type: None,
                    discriminator_type: None,
                    bound: Vec::new(),
                    element_type: None,
                    key_element_type: None,
                    extensibility_kind: ExtensibilityKind::Final,
                    is_nested: false,
                });
                builder
                    .add_member(MemberDescriptor {
                        name: "maybe_a".to_string(),
                        id: 0,
                        r#type: u32::get_dynamic_type(),
                        default_value: None,
                        index: 0,
                        label: Vec::new(),
                        try_construct_kind: TryConstructKind::UseDefault,
                        key: None,
                        is_optional: true,
                        is_must_understand: false,
                        is_shared: false,
                        is_default_label: false,
                    })
                    .expect("`maybe_a` must be a valid descriptor");
                builder
                    .add_member(MemberDescriptor {
                        name: "maybe_b".to_string(),
                        id: 1,
                        r#type: f64::get_dynamic_type(),
                        default_value: None,
                        index: 1,
                        label: Vec::new(),
                        try_construct_kind: TryConstructKind::UseDefault,
                        key: None,
                        is_optional: true,
                        is_must_understand: false,
                        is_shared: false,
                        is_default_label: false,
                    })
                    .expect("`maybe_b` must be a valid descriptor");
                builder
                    .add_member(MemberDescriptor {
                        name: "maybe_c".to_string(),
                        id: 2,
                        r#type: String::get_dynamic_type(),
                        default_value: None,
                        index: 2,
                        label: Vec::new(),
                        try_construct_kind: TryConstructKind::UseDefault,
                        key: None,
                        is_optional: true,
                        is_must_understand: false,
                        is_shared: false,
                        is_default_label: false,
                    })
                    .expect("`maybe_c` must be a valid descriptor");

                builder.build()
            }

            fn create_sample(mut src: DynamicData) -> Self {
                let maybe_a = src.remove_value(0).ok().map(|data_storage| {
                    DataStorageMapping::try_from_storage(data_storage)
                        .expect("`maybe_a` must match")
                });
                let maybe_b = src.remove_value(1).ok().map(|data_storage| {
                    DataStorageMapping::try_from_storage(data_storage)
                        .expect("`maybe_b` must match")
                });
                let maybe_c = src.remove_value(2).ok().map(|data_storage| {
                    DataStorageMapping::try_from_storage(data_storage)
                        .expect("`maybe_c` must match")
                });

                Self {
                    maybe_a,
                    maybe_b,
                    maybe_c,
                }
            }

            fn create_dynamic_sample(self) -> DynamicData {
                let mut data = DynamicDataFactory::create_data(Self::get_type());
                if let Some(maybe_a) = self.maybe_a {
                    data.set_value(0, maybe_a.into_storage());
                }
                if let Some(maybe_b) = self.maybe_b {
                    data.set_value(1, maybe_b.into_storage());
                }
                if let Some(maybe_c) = self.maybe_c {
                    data.set_value(2, maybe_c.into_storage());
                }
                data
            }
        }
    }
}

fn main() {
    let domain_id = 0;
    let participant_factory = DomainParticipantFactory::get_instance();

    let participant = participant_factory
        .create_participant(domain_id, QosKind::Default, NO_LISTENER, NO_STATUS)
        .unwrap();

    let topic = participant
        .create_topic::<OptionalType>(
            "Optional",
            OptionalType::get_type_name(),
            QosKind::Default,
            NO_LISTENER,
            NO_STATUS,
        )
        .unwrap();

    let publisher = participant
        .create_publisher(QosKind::Default, NO_LISTENER, NO_STATUS)
        .unwrap();

    let writer_qos = DataWriterQos {
        reliability: ReliabilityQosPolicy {
            kind: ReliabilityQosPolicyKind::Reliable,
            max_blocking_time: DurationKind::Finite(Duration::new(1, 0)),
        },
        durability: DurabilityQosPolicy {
            kind: DurabilityQosPolicyKind::TransientLocal,
        },
        ..Default::default()
    };
    let writer = publisher
        .create_datawriter(
            &topic,
            QosKind::Specific(writer_qos),
            NO_LISTENER,
            NO_STATUS,
        )
        .unwrap();
    let writer_cond = writer.get_statuscondition();
    writer_cond
        .set_enabled_statuses(&[StatusKind::PublicationMatched])
        .unwrap();
    let mut wait_set = WaitSet::new();
    wait_set
        .attach_condition(Condition::StatusCondition(writer_cond))
        .unwrap();

    wait_set.wait(Duration::new(60, 0)).unwrap();

    let data = OptionalType {
        maybe_a: None,
        maybe_b: Some(1234.56789),
        maybe_c: None,
    };
    println!("write: {data:?}");
    writer.write(data, None).unwrap();

    writer
        .wait_for_acknowledgments(Duration::new(30, 0))
        .unwrap();
}
