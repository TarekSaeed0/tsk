#include <tsk/map.h>

#include <stdio.h>
#include <string.h>

static const TskTraitComplete string_trait_complete = {
	.size      = sizeof(const TskChar *),
	.alignment = alignof(const TskChar *),
};

static const TskTraitDroppable string_trait_droppable = {
	.drop = TSK_NULL,
};

static const TskTraitClonable string_trait_clonable = {
	.trait_complete = &string_trait_complete,
	.clone          = TSK_NULL,
};

static TskBool string_type_equatable_equals(const TskAny *string_1, const TskAny *string_2) {
	return strcmp(*(const TskChar **)string_1, *(const TskChar **)string_2) == 0;
}
static const TskTraitEquatable string_trait_equatable = {
	.equals = string_type_equatable_equals,
};

static TskEmpty string_trait_hashable_hash(const TskAny *string, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	TskUSize length = strlen(*(const TskChar **)string);
	tsk_trait_hasher_combine(
	    trait_hasher,
	    hasher,
	    (const TskU8 *)*(const TskChar **)string,
	    length
	);
}
static const TskTraitHashable string_trait_hashable = {
	.hash = string_trait_hashable_hash,
};

const TskType string_type = {
	.name   = "const TskChar *",
	.traits = {
	    [TSK_TRAIT_COMPLETE]  = &string_trait_complete,
	    [TSK_TRAIT_DROPPABLE] = &string_trait_droppable,
	    [TSK_TRAIT_CLONABLE]  = &string_trait_clonable,
	    [TSK_TRAIT_EQUATABLE] = &string_trait_equatable,
	    [TSK_TRAIT_HASHABLE]  = &string_trait_hashable,
	},
};

int main(void) {
	char text[]               = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus maximus sem sit amet sapien faucibus condimentum. Aenean sed nunc pretium, lobortis purus vitae, hendrerit mauris. Maecenas ut nisl vitae nibh dignissim eleifend vel at ante. In tellus elit, ullamcorper at consectetur at, facilisis eget ante. Mauris egestas facilisis enim, eu semper turpis dictum sed. Donec sed tristique neque. Nullam imperdiet porta aliquam. In hac habitasse platea dictumst. Pellentesque dictum a enim id dictum. Integer leo ante, dictum vel euismod a, pellentesque et nisi. Integer ac elementum velit, vel dignissim mauris. Vestibulum ac pellentesque libero. Pellentesque eget eros varius, mollis velit vitae, ornare turpis.\n\
Integer leo nunc, gravida ut odio ac, tincidunt placerat quam. Nullam laoreet, arcu in pretium gravida, purus odio hendrerit tortor, fermentum euismod ligula lacus ut massa. Curabitur eget lectus volutpat, hendrerit augue eget, varius libero. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Donec sapien elit, efficitur fringilla risus vitae, posuere finibus purus. Suspendisse fringilla nunc nec cursus consectetur. Integer placerat urna lorem, eget consequat libero feugiat eu. Quisque scelerisque ornare nisl id pretium. In consectetur aliquet magna ut molestie. Sed id neque et mauris efficitur porta. Aliquam eu turpis posuere, volutpat nisl vel, pellentesque nulla. Donec semper ligula eu metus iaculis, in congue massa malesuada. Sed fermentum lobortis enim a luctus. Nam porttitor, neque et hendrerit euismod, ante tortor pretium lacus, in varius metus est id felis. Phasellus vitae egestas risus.\n\
Sed pulvinar velit a elit vehicula, eget placerat nulla sollicitudin. Curabitur tempor, turpis eu ullamcorper lacinia, nibh nibh venenatis mi, vitae egestas nisl arcu eget erat. Mauris euismod dui ut nunc interdum vestibulum. Quisque ligula est, tristique quis fermentum ut, placerat quis ipsum. Fusce hendrerit sapien vel tellus vestibulum mollis. Nulla facilisi. Sed malesuada varius mollis. Ut volutpat placerat mollis. Quisque mi ligula, efficitur in nulla id, vestibulum vestibulum metus. Suspendisse efficitur justo porttitor tortor feugiat, eget viverra augue convallis. Donec quis commodo arcu, eu pellentesque justo. Nam laoreet lobortis magna nec lacinia. Donec convallis tempor lorem, dapibus ultricies diam iaculis vitae. Aenean ac enim ut elit iaculis vulputate ut sit amet nulla. Cras scelerisque dolor eget dui interdum congue. Duis iaculis hendrerit urna, eget pretium velit scelerisque a.\n\
In efficitur accumsan fringilla. Phasellus pretium finibus nunc at feugiat. Curabitur pulvinar felis ac nunc iaculis viverra. Donec faucibus accumsan enim non molestie. Praesent facilisis, enim eget finibus convallis, ex turpis fermentum sapien, ac aliquam enim erat rutrum metus. Ut metus lorem, laoreet eget purus in, auctor semper mi. Pellentesque nisi diam, rutrum vitae nisi id, vestibulum aliquam orci. Fusce ut urna neque.\n\
Nullam lacinia urna ligula, at vehicula dolor facilisis ac. Vestibulum ut lorem eleifend metus egestas laoreet quis quis elit. In aliquet volutpat elit porta pulvinar. Aliquam sed risus non ipsum fringilla vehicula. Nunc ligula mauris, sagittis non justo at, eleifend imperdiet turpis. Duis ac arcu vitae risus rutrum egestas. Suspendisse ornare at neque ut ultrices. Morbi eu dignissim neque.";

	TskMap         words      = tsk_map_new(&string_type, &tsk_unit_type);
	const TskChar *delimiters = " \n\t.,;:!?\"'()[]{}<>";
	TskChar       *token      = strtok(text, delimiters);
	while (token != TSK_NULL) {
		tsk_map_insert(&words, &token, &tsk_unit);
		token = strtok(TSK_NULL, delimiters);
	}

	TskMapIteratorItem item;
	for (TskMapIterator iterator = tsk_map_iterator(&words); tsk_map_iterator_next(&iterator, &item);) {
		printf("%s\n", *(const TskChar **)item.key);
	}

	tsk_map_drop(&words);
}
