format 213

classcanvas 128002 class_ref 425730 // QCoreApplication
  classdiagramsettings member_max_width 0 end
  xyz 39.75 195.25 2000
end
classcanvas 128258 class_ref 4162050 // AbstractConsoleApplicationMainFunction
  classdiagramsettings member_max_width 0 end
  xyz 382 165.5 2000
end
classcanvas 128514 class_ref 138626 // QObject
  classdiagramsettings hide_attributes yes hide_operations yes member_max_width 0 end
  xyz 489 40.75 2005
end
relationcanvas 128642 relation_ref 4659202 // <generalisation>
  geometry VHV unfixed
  from ref 128002 z 2006 to point 190 125
  line 128898 z 2006 to point 519 125
  line 129026 z 2006 to ref 128514
  no_role_a no_role_b
  no_multiplicity_a no_multiplicity_b
end
relationcanvas 128770 relation_ref 4659330 // <generalisation>
  from ref 128258 z 2006 to ref 128514
  no_role_a no_role_b
  no_multiplicity_a no_multiplicity_b
end
relationcanvas 129154 relation_ref 4659458 // <dependency>
  decenter_end 557
  from ref 128258 z 2001 to ref 128002
  no_role_a no_role_b
  no_multiplicity_a no_multiplicity_b
end
end
