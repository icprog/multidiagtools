format 76

classcanvas 128002 class_ref 917122 // mdtSqlDatabaseManager
  draw_all_relations default hide_attributes yes hide_operations yes hide_getset_operations default show_members_full_definition default show_members_visibility default show_members_stereotype default show_members_context default show_members_multiplicity default show_members_initialization default show_attribute_modifiers default member_max_width 0 show_parameter_dir default show_parameter_name default package_name_in_tab default class_drawing_mode default drawing_language default show_context_mode default auto_label_position default show_relation_modifiers default show_relation_visibility default show_infonote default shadow default show_stereotype_properties default
  xyz 51 201 2000
end
classcanvas 128130 class_ref 910210 // mdtSqlSchemaTable
  draw_all_relations default hide_attributes yes hide_operations yes hide_getset_operations default show_members_full_definition default show_members_visibility default show_members_stereotype default show_members_context default show_members_multiplicity default show_members_initialization default show_attribute_modifiers default member_max_width 0 show_parameter_dir default show_parameter_name default package_name_in_tab default class_drawing_mode default drawing_language default show_context_mode default auto_label_position default show_relation_modifiers default show_relation_visibility default show_infonote default shadow default show_stereotype_properties default
  xyz 494 214 2000
end
classcanvas 128258 class_ref 923778 // mdtTtDatabaseSchema
  draw_all_relations default hide_attributes default hide_operations default hide_getset_operations default show_members_full_definition default show_members_visibility default show_members_stereotype default show_members_context default show_members_multiplicity default show_members_initialization default show_attribute_modifiers default member_max_width 0 show_parameter_dir default show_parameter_name default package_name_in_tab default class_drawing_mode default drawing_language default show_context_mode default auto_label_position default show_relation_modifiers default show_relation_visibility default show_infonote default shadow default show_stereotype_properties default
  xyz 209 335 2000
end
relationcanvas 128386 relation_ref 1132802 // <directional composition>
  from ref 128258 z 2001 stereotype "<<list>>" xyz 514 294 3000 to ref 128130
  role_a_pos 571 262 3000 no_role_b
  multiplicity_a_pos 544 262 3000 no_multiplicity_b
end
relationcanvas 128514 relation_ref 1132930 // <directional composition>
  from ref 128258 z 2001 to ref 128002
  role_a_pos 176 249 3000 no_role_b
  multiplicity_a_pos 148 249 3000 no_multiplicity_b
end
end
