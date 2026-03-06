// World and spatial query utilities.
// Wraps QueryEntitiesBySphere and TraceMove for common use cases.

class EIR_WorldUtils
{
	// Internal accumulator used by the sphere query callback.
	protected static ref array<IEntity> s_QueryResults;

	//------------------------------------------------------------------------------------------------
	// Fills outEntities with all entities within radius of origin.
	// flags defaults to all entity types; pass EQueryEntitiesFlags to narrow.
	// Returns the count found.
	static int FindEntitiesInRadius(vector origin, float radius, out array<IEntity> outEntities, EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL)
	{
		if (!outEntities)
			outEntities = {};

		outEntities.Clear();

		s_QueryResults = outEntities;
		GetGame().GetWorld().QueryEntitiesBySphere(origin, radius, CollectEntity, null, flags);
		s_QueryResults = null;

		return outEntities.Count();
	}

	//------------------------------------------------------------------------------------------------
	// Returns the closest entity within radius of origin, or null if none found.
	static IEntity FindClosestEntityInRadius(vector origin, float radius, EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL)
	{
		array<IEntity> found = {};
		FindEntitiesInRadius(origin, radius, found, flags);

		IEntity closest = null;
		float closestDistSq = float.MAX;

		foreach (IEntity e : found)
		{
			float distSq = vector.DistanceSq(e.GetOrigin(), origin);
			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;
				closest = e;
			}
		}

		return closest;
	}

	//------------------------------------------------------------------------------------------------
	// Returns the Y (height) of the terrain or surface directly below pos.
	// Traces downward from pos + traceHeight. Returns pos[1] if nothing is hit.
	static float GetSurfaceY(vector pos, float traceHeight = 500.0)
	{
		vector traceStart = Vector(pos[0], pos[1] + traceHeight, pos[2]);
		vector traceEnd = Vector(pos[0], pos[1] - traceHeight, pos[2]);

		TraceParam trace = new TraceParam();
		trace.Start = traceStart;
		trace.End = traceEnd;
		trace.Flags = TraceFlags.WORLD;

		float result = GetGame().GetWorld().TraceMove(trace, null);

		// result == 1.0 means no hit
		if (result >= 1.0)
			return pos[1];

		return traceStart[1] + (traceEnd[1] - traceStart[1]) * result;
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if there is an unobstructed line of sight between from and to.
	// exclude is typically the entity performing the check.
	static bool HasLineOfSight(vector from, vector to, IEntity exclude = null)
	{
		TraceParam trace = new TraceParam();
		trace.Start = from;
		trace.End = to;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.Exclude = exclude;

		float result = GetGame().GetWorld().TraceMove(trace, null);

		// result == 1.0 means the ray travelled the full distance without hitting anything
		return result >= 1.0;
	}

	//------------------------------------------------------------------------------------------------
	// Returns the hit position of a ray from start toward end.
	// Returns end unchanged if nothing is hit.
	static vector TracePosition(vector start, vector end, IEntity exclude = null)
	{
		TraceParam trace = new TraceParam();
		trace.Start = start;
		trace.End = end;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.Exclude = exclude;

		float result = GetGame().GetWorld().TraceMove(trace, null);

		return start + (end - start) * result;
	}

	//------------------------------------------------------------------------------------------------
	// Internal callback used by FindEntitiesInRadius.
	protected static bool CollectEntity(IEntity entity)
	{
		if (s_QueryResults)
			s_QueryResults.Insert(entity);

		return true;
	}
}
