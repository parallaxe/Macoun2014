#define EMBARRASING_LONG_DEFINE NSLog(@"oh man, this define is sooooooo long, it really should be splitted into multiple lines")

enum   OptionsWithLongNames {
 OptionsWithLongNamesTransparentBlackDeprecated, OptionsWithLongNamesDefault
};

@interface BaseClassWithALongName
  -(void )  thisMethodDoesItAll:(enum OptionsWithLongNames)itAll andEvenMore:(BOOL)andMore andSoOn:(BOOL)andSoOn;
@end

@implementation BaseClassWithALongName
-(void)thisMethodDoesItAll:(enum OptionsWithLongNames)itAll andEvenMore:( BOOL)andMore andSoOn:(BOOL)andSoOn 
{
	// this comment explains in an extraordinary way what the heck the following lines do
  if( andMore ) 
	{
		if(andSoOn) {
			if(itAll== OptionsWithLongNamesTransparentBlackDeprecated) {
					NSLog(@"three nested ifs - who in the world does that?");
				}
		}
	}
}
@end

