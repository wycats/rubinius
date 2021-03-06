#include "vm/test/test.hpp"

#include "builtin/regexp.hpp"

class TestRegexp : public CxxTest::TestSuite, public VMTest {
public:

  void setUp() {
    create();
  }

  void tearDown() {
    destroy();
  }

  void test_version() {
    char* ver = Regexp::version(state);
    TS_ASSERT(strlen(ver) > 0);
  }

  void test_create() {
    Regexp* re = Regexp::create(state);
    TS_ASSERT(re->source()->nil_p());
    TS_ASSERT(re->names()->nil_p());
    TS_ASSERT_EQUALS(re->klass(), G(regexp));
  }

  void test_initialize() {
    String *pat = String::create(state, ".");
    Regexp* re = Regexp::create(state);
    re->initialize(state, pat, Fixnum::from(0), Qnil);

    TS_ASSERT_EQUALS(re->source(), pat);
    TS_ASSERT_EQUALS(re->names(),  Qnil);
  }

  void test_allocate() {
    Class* sub = state->new_class("RegexpSub", G(regexp), 0);
    Regexp* re = Regexp::allocate(state, sub);

    TS_ASSERT_EQUALS(re->klass(), sub);
  }

  void test_create_with_named_captures() {
    String *pat = String::create(state, "(?<blah>.)");
    Regexp* re = Regexp::create(state);
    re->initialize(state, pat, Fixnum::from(0), Qnil);

    TS_ASSERT_EQUALS(re->source(), pat);
    TS_ASSERT(re->names()->kind_of_p(state, G(lookuptable)));
  }

  void test_create_with_bad_pattern() {
    char buf[1024];
    std::memset(buf, 0, 1024);

    String *pat = String::create(state, "(?");
    Regexp* re = Regexp::create(state);
    TS_ASSERT_THROWS(re->initialize(state, pat, Fixnum::from(0), Qnil),
                     const RubyException &);
  }

  void test_options() {
    String *pat = String::create(state, ".");
    Regexp* re = Regexp::create(state);
    re->initialize(state, pat, Fixnum::from(0), Qnil);

    TS_ASSERT_EQUALS(as<Integer>(re->options(state))->to_native(), 16);
  }

  void test_match_region() {
    String *pat = String::create(state, ".");
    Regexp* re = Regexp::create(state);
    re->initialize(state, pat, Fixnum::from(0), Qnil);

    String *input = String::create(state, "abc");

    Fixnum* start = Fixnum::from(0);
    Fixnum* end =   Fixnum::from(3);
    Object* forward = Qtrue;

    MatchData* matches = (MatchData*)re->match_region(state, input, start, end, forward);
    TS_ASSERT(!matches->nil_p());
    TS_ASSERT_EQUALS(matches->region()->num_fields(), 0U);
    TS_ASSERT_EQUALS(as<Integer>(matches->full()->at(state, 0))->to_native(), 0);
    TS_ASSERT_EQUALS(as<Integer>(matches->full()->at(state, 1))->to_native(), 1);
  }

  void test_match_region_without_matches() {
    String *pat = String::create(state, "d");
    Regexp* re = Regexp::create(state);
    re->initialize(state, pat, Fixnum::from(0), Qnil);

    String *input = String::create(state, "abc");

    Fixnum* start = Fixnum::from(0);
    Fixnum* end =   Fixnum::from(3);
    Object* forward = Qtrue;

    MatchData* matches = (MatchData*)re->match_region(state, input, start, end, forward);
    TS_ASSERT(matches->nil_p());
  }

  void test_match_region_with_captures() {
    String *pat = String::create(state, ".(.)");
    Regexp* re = Regexp::create(state);
    re->initialize(state, pat, Fixnum::from(0), Qnil);

    String *input = String::create(state, "abc");

    Fixnum* start = Fixnum::from(0);
    Fixnum* end =   Fixnum::from(3);
    Object* forward = Qtrue;

    MatchData* matches = (MatchData*)re->match_region(state, input, start, end, forward);
    TS_ASSERT(!matches->nil_p());
    TS_ASSERT_EQUALS(as<Integer>(matches->full()->at(state, 0))->to_native(), 0);
    TS_ASSERT_EQUALS(as<Integer>(matches->full()->at(state, 1))->to_native(), 2);

    TS_ASSERT_EQUALS(matches->region()->num_fields(), 1U);
    TS_ASSERT_EQUALS(as<Integer>(as<Tuple>(matches->region()->at(state, 0))->at(state, 0))->to_native(), 1);
    TS_ASSERT_EQUALS(as<Integer>(as<Tuple>(matches->region()->at(state, 0))->at(state, 1))->to_native(), 2);
  }

  void test_match_region_with_backward_captures() {
    String *pat = String::create(state, ".(.)");
    Regexp* re = Regexp::create(state);
    re->initialize(state, pat, Fixnum::from(0), Qnil);

    String *input = String::create(state, "abc");

    Fixnum* start = Fixnum::from(0);
    Fixnum* end =   Fixnum::from(3);
    Object* forward = Qfalse;

    MatchData* matches = (MatchData*)re->match_region(state, input, start, end, forward);
    TS_ASSERT(!matches->nil_p());
    TS_ASSERT_EQUALS(as<Integer>(matches->full()->at(state, 0))->to_native(), 1);
    TS_ASSERT_EQUALS(as<Integer>(matches->full()->at(state, 1))->to_native(), 3);

    TS_ASSERT_EQUALS(matches->region()->num_fields(), 1U);
    TS_ASSERT_EQUALS(as<Integer>(as<Tuple>(matches->region()->at(state, 0))->at(state, 0))->to_native(), 2);
    TS_ASSERT_EQUALS(as<Integer>(as<Tuple>(matches->region()->at(state, 0))->at(state, 1))->to_native(), 3);
  }

  void test_match_start() {
    String *pat = String::create(state, ".");
    Regexp* re = Regexp::create(state);
    re->initialize(state, pat, Fixnum::from(0), Qnil);

    String *input = String::create(state, "abc");

    Fixnum* start = Fixnum::from(1);

    MatchData* matches = (MatchData*)re->match_start(state, input, start);
    TS_ASSERT(!matches->nil_p());
    TS_ASSERT_EQUALS(matches->region()->num_fields(), 0U);
    TS_ASSERT_EQUALS(as<Integer>(matches->full()->at(state, 0))->to_native(), 1);
    TS_ASSERT_EQUALS(as<Integer>(matches->full()->at(state, 1))->to_native(), 2);
  }

};
